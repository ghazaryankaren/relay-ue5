#include "Router/RelayScreen.h"

#include "CommonInputModeTypes.h"
#include "Engine/LocalPlayer.h"
#include "Input/CommonUIInputTypes.h"
#include "Input/UIActionBindingHandle.h"
#include "InputAction.h"
#include "Router/RelayUIRouterSubsystem.h"

URelayScreen::URelayScreen(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	// Back закрывает любой экран Relay через роутер. Экран, которому это не нужно (HUD), выключает флаг в WBP
	bIsBackHandler = true;

	// При возврате на экран фокус встаёт туда, где был, а не на виджет по умолчанию
	bAutoRestoreFocus = true;
}

TOptional<FUIInputConfig> URelayScreen::GetDesiredInputConfig() const
{
	if (InputMode == ERelayScreenInputMode::Game)
	{
		// Именно CapturePermanently, как в Lyra. С CapturePermanently_IncludingInitialMouseDown аналоговый курсор
		// CommonUI считает, что может отпустить захват мыши, и в игре превращает кнопку A геймпада в клик мышью:
		// нажатие съедается и не доходит до прыжка (CommonAnalogCursor.cpp, CanReleaseMouseCapture и IsRelevantInput)
		return FUIInputConfig(ECommonInputMode::Game, EMouseCaptureMode::CapturePermanently);
	}

	return FUIInputConfig(ECommonInputMode::Menu, EMouseCaptureMode::NoCapture);
}

void URelayScreen::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	// Привязки регистрируются один раз на объект. UCommonUserWidget хранит их хэндлы и сам переподключает
	// при каждой перестройке виджета, а NativeDestruct их только отсоединяет. Повторная регистрация
	// в NativeConstruct у экрана из пула падала бы с ошибкой "already bound" при каждом показе.
	// Привязка срабатывает только в режиме ввода экрана: в HUD (Game) - когда меню закрыты
	const ECommonInputMode BindingMode = InputMode == ERelayScreenInputMode::Game ? ECommonInputMode::Game : ECommonInputMode::Menu;

	for (const FRelayScreenShortcut& Shortcut : Shortcuts)
	{
		if (!Shortcut.Action || !Shortcut.Screen.IsValid())
		{
			continue;
		}

		FBindUIActionArgs BindArgs(Shortcut.Action, false, FSimpleDelegate::CreateUObject(this, &URelayScreen::HandleShortcut, Shortcut.Screen));
		BindArgs.InputMode = BindingMode;
		RegisterUIActionBinding(BindArgs);
	}
}

bool URelayScreen::NativeOnHandleBackAction()
{
	if (!bIsBackHandler)
	{
		return false;
	}

	if (URelayUIRouterSubsystem* Router = GetRouter())
	{
		Router->RequestCloseScreen(*this);
		return true;
	}

	return Super::NativeOnHandleBackAction();
}

FNavigationReply URelayScreen::NativeOnNavigation(const FGeometry& MyGeometry, const FNavigationEvent& InNavigationEvent, const FNavigationReply& InDefaultReply)
{
	const FNavigationReply Reply = Super::NativeOnNavigation(MyGeometry, InNavigationEvent, InDefaultReply);

	// Навигация Slate геометрическая и про экраны CommonUI не знает: из диалога вниз фокус уходил на кнопку Back
	// магазина под ним, и Back закрывал магазин из-под открытого диалога. Stop делает этот экран границей:
	// Slate ищет следующий виджет только среди его потомков (обработчик ответа - этот экран, SlateApplication.cpp:3688)
	if (Reply.GetBoundaryRule() == EUINavigationRule::Escape)
	{
		return FNavigationReply::Stop();
	}

	return Reply;
}

URelayUIRouterSubsystem* URelayScreen::GetRouter() const
{
	return ULocalPlayer::GetSubsystem<URelayUIRouterSubsystem>(GetOwningLocalPlayer());
}

void URelayScreen::HandleShortcut(FGameplayTag ScreenTag)
{
	if (URelayUIRouterSubsystem* Router = GetRouter())
	{
		Router->PushScreen(ScreenTag);
	}
}
