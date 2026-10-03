#include "Router/RelayUIRouterSubsystem.h"

#include "Blueprint/UserWidget.h"
#include "CommonInputModeTypes.h"
#include "Engine/AssetManager.h"
#include "Engine/DebugCameraController.h"
#include "Engine/LocalPlayer.h"
#include "Engine/StreamableManager.h"
#include "EnhancedInputSubsystems.h"
#include "Framework/Application/SlateApplication.h"
#include "Framework/Application/SlateUser.h"
#include "GameFramework/PlayerController.h"
#include "Input/CommonUIActionRouterBase.h"
#include "InputMappingContext.h"
#include "RelayUILog.h"
#include "RelayUITags.h"
#include "RelayUISettings.h"
#include "Router/RelayRootLayout.h"
#include "Router/RelayScreen.h"
#include "Router/RelayScreenRegistry.h"
#include "Widgets/CommonActivatableWidgetContainer.h"

void URelayUIRouterSubsystem::PlayerControllerChanged(APlayerController* NewPlayerController)
{
	Super::PlayerControllerChanged(NewPlayerController);

	// В Initialize контроллера и места во вьюпорте ещё нет, поэтому UI строится здесь.
	// Отладочная камера временно подменяет контроллер: UI для неё не строим, иначе переключение камеры
	// сбрасывало бы весь стек на стартовые экраны
	if (!NewPlayerController || !NewPlayerController->IsLocalController() || NewPlayerController->IsA<ADebugCameraController>())
	{
		return;
	}

	// Контекст IMC_UI и теги режима ввода живут на PlayerInput контроллера, поэтому их получает
	// каждый новый контроллер, даже если UI пересобирать не нужно
	SetUpPlayerInput();

	// Вернулись к контроллеру, для которого UI уже на экране (например, после отладочной камеры)
	if (RootLayout && LayoutController.Get() == NewPlayerController && RootLayout->IsInViewport())
	{
		return;
	}

	if (!CreateRootLayout(NewPlayerController))
	{
		return;
	}

	LayoutController = NewPlayerController;

	for (const FGameplayTag& ScreenTag : GetDefault<URelayUISettings>()->InitialScreens)
	{
		PushScreen(ScreenTag);
	}
}

void URelayUIRouterSubsystem::Deinitialize()
{
	DestroyRootLayout();
	ScreenRegistry = nullptr;

	Super::Deinitialize();
}

void URelayUIRouterSubsystem::PushScreen(FGameplayTag ScreenTag)
{
	OpenScreen(ScreenTag, EScreenOp::Push);
}

void URelayUIRouterSubsystem::ReplaceScreen(FGameplayTag ScreenTag)
{
	OpenScreen(ScreenTag, EScreenOp::Replace);
}

void URelayUIRouterSubsystem::RequestCloseScreen(URelayScreen& Screen)
{
	if (!IsUIOnScreen() || !Screen.IsActivated())
	{
		return;
	}

	if (!Screen.HandleCloseRequest())
	{
		UE_LOG(LogRelayUI, Log, TEXT("%s refused to close."), *Screen.GetName());
		return;
	}

	// Пользователь ушёл назад раньше, чем догрузился следующий экран: этот экран ему уже не нужен.
	// Отменяем и слои выше: модалка, которую запросило закрываемое меню, тоже больше не нужна
	const FGameplayTag LayerTag = RootLayout->FindLayerTagOf(Screen);
	const TArray<FGameplayTag>& Layers = RelayUITags::AllLayers();
	const int32 LayerIndex = Layers.IndexOfByKey(LayerTag);
	if (LayerIndex != INDEX_NONE)
	{
		for (int32 Index = LayerIndex; Index < Layers.Num(); ++Index)
		{
			CancelPendingScreen(Layers[Index]);
		}
	}

	// HUD - нижний экран, на который возвращается управление. Если он не открылся, после закрытия
	// последнего меню CommonUI оставит режим Menu, и игрок останется без управления и без меню
	if (!HasOtherActiveScreen(Screen))
	{
		UE_LOG(LogRelayUI, Warning, TEXT("Closing %s leaves no screen open: input stays in the last mode. Check that the HUD screen opened."), *Screen.GetName());
	}

	// Стек CommonUI снимает деактивированный верхний экран и активирует экран под ним
	Screen.DeactivateWidget();
	UE_LOG(LogRelayUI, Log, TEXT("Closed %s on %s."), *Screen.GetName(), *LayerTag.ToString());
}

void URelayUIRouterSubsystem::RequestCloseTopScreen(FGameplayTag LayerTag)
{
	if (!IsUIOnScreen())
	{
		return;
	}

	const UCommonActivatableWidgetContainerBase* Layer = RootLayout->FindLayer(LayerTag);
	if (!Layer)
	{
		UE_LOG(LogRelayUI, Warning, TEXT("RequestCloseTopScreen(%s): layer is not in the root layout."), *LayerTag.ToString());
		return;
	}

	URelayScreen* TopScreen = Cast<URelayScreen>(Layer->GetActiveWidget());
	if (TopScreen)
	{
		RequestCloseScreen(*TopScreen);
	}
	else
	{
		CancelPendingScreen(LayerTag);
	}
}

void URelayUIRouterSubsystem::OpenScreen(FGameplayTag ScreenTag, EScreenOp Op)
{
	// После выгрузки мира лейаут снят с экрана, но объект ещё жив до следующего контроллера:
	// экран, добавленный в такой стек, потерялся бы без ошибки
	if (!IsUIOnScreen())
	{
		UE_LOG(LogRelayUI, Warning, TEXT("OpenScreen(%s): UI is not on screen."), *ScreenTag.ToString());
		return;
	}

	const FRelayScreenEntry* Entry = ScreenRegistry->FindScreen(ScreenTag);
	if (!Entry)
	{
		UE_LOG(LogRelayUI, Error, TEXT("OpenScreen(%s): screen is not in the registry."), *ScreenTag.ToString());
		return;
	}

	if (!RootLayout->FindLayer(Entry->Layer))
	{
		UE_LOG(LogRelayUI, Error, TEXT("OpenScreen(%s): layer %s is not in the root layout."), *ScreenTag.ToString(), *Entry->Layer.ToString());
		return;
	}

	if (Entry->WidgetClass.IsNull())
	{
		UE_LOG(LogRelayUI, Error, TEXT("OpenScreen(%s): screen has no widget class."), *ScreenTag.ToString());
		return;
	}

	// На слое не больше одного ожидающего экрана: новый запрос отменяет старый
	CancelPendingScreen(Entry->Layer);

	// Класс уже в памяти: открываем сразу. Колбэк загрузки пришёл бы только на следующем кадре
	if (UClass* LoadedClass = Entry->WidgetClass.Get())
	{
		ShowScreen(ScreenTag, *Entry, LoadedClass, Op);
		return;
	}

	const FGameplayTag LayerTag = Entry->Layer;
	const uint32 RequestId = ++LastRequestId;

	// Запись создаётся до запроса: при s.StreamableDelegateDelayFrames=0 колбэк может прийти прямо внутри RequestAsyncLoad
	FPendingScreen& NewPending = PendingScreens.Add(LayerTag);
	NewPending.ScreenTag = ScreenTag;
	NewPending.Op = Op;
	NewPending.RequestId = RequestId;

	TSharedPtr<FStreamableHandle> Handle = UAssetManager::GetStreamableManager().RequestAsyncLoad(
		Entry->WidgetClass.ToSoftObjectPath(),
		FStreamableDelegate::CreateWeakLambda(this, [this, LayerTag, RequestId]()
		{
			OnScreenClassLoaded(LayerTag, RequestId);
		}),
		FStreamableManager::AsyncLoadHighPriority);

	// Ссылку NewPending повторно не используем: колбэк мог уже удалить запись из карты
	FPendingScreen* Pending = PendingScreens.Find(LayerTag);
	if (Pending && Pending->RequestId == RequestId)
	{
		if (Handle.IsValid())
		{
			Pending->Handle = MoveTemp(Handle);
		}
		else
		{
			PendingScreens.Remove(LayerTag);
			UE_LOG(LogRelayUI, Error, TEXT("OpenScreen(%s): failed to start loading %s."), *ScreenTag.ToString(), *Entry->WidgetClass.ToString());
			return;
		}
	}

	UE_LOG(LogRelayUI, Verbose, TEXT("Loading %s for %s."), *Entry->WidgetClass.ToString(), *ScreenTag.ToString());
}

void URelayUIRouterSubsystem::OnScreenClassLoaded(FGameplayTag LayerTag, uint32 RequestId)
{
	// Запрос мог устареть, пока класс грузился: его заменил новый запрос на этот слой, отменил Back
	// или UI пересобрали для нового контроллера. Отмена гасит колбэк через CancelHandle,
	// а сверка номера запроса - вторая линия защиты на случай, если колбэк всё же пришёл
	const FPendingScreen* Pending = PendingScreens.Find(LayerTag);
	if (!Pending || Pending->RequestId != RequestId)
	{
		return;
	}

	const FGameplayTag ScreenTag = Pending->ScreenTag;
	const EScreenOp Op = Pending->Op;
	PendingScreens.Remove(LayerTag);

	if (!IsUIOnScreen())
	{
		return;
	}

	const FRelayScreenEntry* Entry = ScreenRegistry->FindScreen(ScreenTag);
	UClass* LoadedClass = Entry ? Entry->WidgetClass.Get() : nullptr;
	if (!LoadedClass)
	{
		UE_LOG(LogRelayUI, Error, TEXT("OpenScreen(%s): failed to load the widget class."), *ScreenTag.ToString());
		return;
	}

	ShowScreen(ScreenTag, *Entry, LoadedClass, Op);
}

void URelayUIRouterSubsystem::ShowScreen(FGameplayTag ScreenTag, const FRelayScreenEntry& Entry, UClass* ScreenClass, EScreenOp Op)
{
	UCommonActivatableWidgetContainerBase* Layer = RootLayout->FindLayer(Entry.Layer);
	if (!Layer)
	{
		return;
	}

	UCommonActivatableWidget* ReplacedScreen = nullptr;
	if (Op == EScreenOp::Replace)
	{
		ReplacedScreen = Layer->GetActiveWidget();

		// Заменяемый экран тоже может отказаться закрыться, как при Back
		URelayScreen* ReplacedRelayScreen = Cast<URelayScreen>(ReplacedScreen);
		if (ReplacedRelayScreen && !ReplacedRelayScreen->HandleCloseRequest())
		{
			UE_LOG(LogRelayUI, Log, TEXT("ReplaceScreen(%s): %s refused to close."), *ScreenTag.ToString(), *ReplacedRelayScreen->GetName());
			return;
		}
	}

	URelayScreen* Screen = Layer->AddWidget<URelayScreen>(ScreenClass);
	if (!Screen)
	{
		UE_LOG(LogRelayUI, Error, TEXT("OpenScreen(%s): failed to create %s."), *ScreenTag.ToString(), *GetNameSafe(ScreenClass));
		return;
	}

	// Сначала новый экран, потом убираем старый. Наоборот нельзя: на мгновение активировался бы
	// экран под старым, сработал бы его OnActivated и прыгнул бы фокус
	if (ReplacedScreen && ReplacedScreen != Screen)
	{
		Layer->RemoveWidget(*ReplacedScreen);
	}

	if (Op == EScreenOp::Replace)
	{
		UE_LOG(LogRelayUI, Log, TEXT("Replaced the top of %s with %s."), *Entry.Layer.ToString(), *ScreenTag.ToString());
	}
	else
	{
		UE_LOG(LogRelayUI, Log, TEXT("Pushed %s to %s."), *ScreenTag.ToString(), *Entry.Layer.ToString());
	}
}

void URelayUIRouterSubsystem::CancelPendingScreen(FGameplayTag LayerTag)
{
	FPendingScreen Pending;
	if (!PendingScreens.RemoveAndCopyValue(LayerTag, Pending))
	{
		return;
	}

	// Только CancelHandle гасит колбэк, который уже стоит в очереди. Если просто отпустить handle,
	// загрузка доиграет и колбэк всё равно придёт
	if (Pending.Handle.IsValid())
	{
		Pending.Handle->CancelHandle();
	}

	UE_LOG(LogRelayUI, Log, TEXT("Cancelled loading of %s on %s."), *Pending.ScreenTag.ToString(), *LayerTag.ToString());
}

void URelayUIRouterSubsystem::CancelAllPendingScreens()
{
	TArray<FGameplayTag> LayerTags;
	PendingScreens.GetKeys(LayerTags);
	for (const FGameplayTag& LayerTag : LayerTags)
	{
		CancelPendingScreen(LayerTag);
	}
}

bool URelayUIRouterSubsystem::CreateRootLayout(APlayerController* PlayerController)
{
	DestroyRootLayout();

	const URelayUISettings* Settings = GetDefault<URelayUISettings>();
	if (Settings->RootLayoutClass.IsNull() || Settings->ScreenRegistry.IsNull())
	{
		UE_LOG(LogRelayUI, Error, TEXT("Root layout class or screen registry is not set: Project Settings > Game > Relay UI."));
		return false;
	}

	// Лейаут и реестр маленькие (в реестре только soft-ссылки) и нужны сразу, поэтому грузятся синхронно
	const TSubclassOf<URelayRootLayout> LayoutClass = Settings->RootLayoutClass.LoadSynchronous();
	if (!LayoutClass)
	{
		UE_LOG(LogRelayUI, Error, TEXT("Failed to load root layout class %s."), *Settings->RootLayoutClass.ToString());
		return false;
	}

	ScreenRegistry = Settings->ScreenRegistry.LoadSynchronous();
	if (!ScreenRegistry)
	{
		UE_LOG(LogRelayUI, Error, TEXT("Failed to load screen registry %s."), *Settings->ScreenRegistry.ToString());
		return false;
	}

	// CreateWidget возвращает nullptr, например, для класса с галкой Abstract
	RootLayout = CreateWidget<URelayRootLayout>(PlayerController, LayoutClass);
	if (!RootLayout)
	{
		UE_LOG(LogRelayUI, Error, TEXT("Failed to create root layout %s."), *GetNameSafe(LayoutClass));
		return false;
	}

	// Сначала во вьюпорт, потом экраны: стек CommonUI без построенного Slate молча теряет добавленный виджет.
	// AddToPlayerScreen может вернуть true, так и не добавив виджет, поэтому проверяем по факту
	RootLayout->AddToPlayerScreen();
	if (!RootLayout->IsInViewport())
	{
		UE_LOG(LogRelayUI, Error, TEXT("Failed to add root layout %s to the player screen."), *GetNameSafe(LayoutClass));
		DestroyRootLayout();
		return false;
	}

#if WITH_EDITOR
	// В PIE CommonUI применяет режим ввода и фокус, только пока окно игры в фокусе Slate. Если Play нажали,
	// когда фокус был в другом окне, меню не отвечало ни клавиатуре, ни геймпаду. Отдаём фокус вьюпорту игры сами
	if (GIsEditor && FSlateApplication::IsInitialized())
	{
		if (const TSharedPtr<FSlateUser> SlateUser = GetLocalPlayer()->GetSlateUser())
		{
			FSlateApplication::Get().SetUserFocusToGameViewport(SlateUser->GetUserIndex());
		}
	}
#endif

	return true;
}

void URelayUIRouterSubsystem::DestroyRootLayout()
{
	// Экраны, которые ещё грузятся для старого лейаута, открывать уже некуда
	CancelAllPendingScreens();

	if (RootLayout)
	{
		RootLayout->RemoveFromParent();
		RootLayout = nullptr;
	}

	LayoutController.Reset();
}

void URelayUIRouterSubsystem::SetUpPlayerInput()
{
	UEnhancedInputLocalPlayerSubsystem* EnhancedInput = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer());
	if (!EnhancedInput)
	{
		return;
	}

	const URelayUISettings* Settings = GetDefault<URelayUISettings>();
	if (Settings->UIMappingContext.IsNull())
	{
		UE_LOG(LogRelayUI, Warning, TEXT("UI mapping context is not set: Back and screen shortcuts will not react to keys."));
	}
	else if (const UInputMappingContext* UIContext = Settings->UIMappingContext.LoadSynchronous())
	{
		// CommonUI ищет клавиши Back только в применённых контекстах, поэтому этот контекст включён всегда
		EnhancedInput->AddMappingContext(UIContext, Settings->UIMappingContextPriority);
	}
	else
	{
		UE_LOG(LogRelayUI, Error, TEXT("Failed to load UI mapping context %s."), *Settings->UIMappingContext.ToString());
	}

	// CommonUI ставит теги InputMode.* в Enhanced Input только при смене режима, а PlayerInput нового контроллера
	// начинается без них: игровые контексты с фильтром InputMode.Game остались бы выключены.
	// Пока CommonUI не применил ни одного режима, теги не ставим: он поставит их сам при первом Game или Menu,
	// а GetActiveInputMode до этого вернул бы All, то есть оба тега сразу
	if (const UCommonUIActionRouterBase* ActionRouter = ULocalPlayer::GetSubsystem<UCommonUIActionRouterBase>(GetLocalPlayer()))
	{
		const ECommonInputMode Mode = ActionRouter->GetActiveInputMode(ECommonInputMode::MAX);
		if (Mode == ECommonInputMode::MAX)
		{
			return;
		}

		FGameplayTagContainer ModeTags;
		if (Mode == ECommonInputMode::Game || Mode == ECommonInputMode::All)
		{
			ModeTags.AddTag(RelayUITags::InputModeGame());
		}
		if (Mode == ECommonInputMode::Menu || Mode == ECommonInputMode::All)
		{
			ModeTags.AddTag(RelayUITags::InputModeMenu());
		}
		EnhancedInput->AppendTagsToInputMode(ModeTags);
	}
}

bool URelayUIRouterSubsystem::IsUIOnScreen() const
{
	return RootLayout && RootLayout->IsInViewport() && ScreenRegistry;
}

bool URelayUIRouterSubsystem::HasOtherActiveScreen(const URelayScreen& Screen) const
{
	for (const FGameplayTag& LayerTag : RelayUITags::AllLayers())
	{
		const UCommonActivatableWidgetContainerBase* Layer = RootLayout->FindLayer(LayerTag);
		if (!Layer)
		{
			continue;
		}

		// На своём слое после закрытия откроется экран под ним, на других слоях важен любой экран
		const int32 ScreensLeft = Layer->GetWidgetList().Contains(&Screen) ? Layer->GetNumWidgets() - 1 : Layer->GetNumWidgets();
		if (ScreensLeft > 0)
		{
			return true;
		}
	}

	return false;
}
