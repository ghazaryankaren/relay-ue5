#include "Focus/RelayFocusSubsystem.h"

#include "CommonActivatableWidget.h"
#include "Engine/LocalPlayer.h"
#include "Focus/RelayPointerMotionTracker.h"
#include "Framework/Application/SlateApplication.h"
#include "Input/CommonUIActionRouterBase.h"
#include "RelayUILog.h"

void URelayFocusSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	if (FSlateApplication::IsInitialized())
	{
		SlateFocusChangingHandle = FSlateApplication::Get().OnFocusChanging().AddUObject(this, &URelayFocusSubsystem::HandleSlateFocusChanging);

		PointerMotionTracker = MakeShared<FRelayPointerMotionTracker>();
		FSlateApplication::Get().RegisterInputPreProcessor(PointerMotionTracker);
	}
}

void URelayFocusSubsystem::Deinitialize()
{
	if (FSlateApplication::IsInitialized())
	{
		FSlateApplication::Get().OnFocusChanging().Remove(SlateFocusChangingHandle);
		if (PointerMotionTracker)
		{
			FSlateApplication::Get().UnregisterInputPreProcessor(PointerMotionTracker);
		}
	}
	SlateFocusChangingHandle.Reset();
	PointerMotionTracker.Reset();

	FTSTicker::RemoveTicker(RefreshTicker);
	RefreshTicker.Reset();

	Super::Deinitialize();
}

void URelayFocusSubsystem::RefreshRememberedFocus() const
{
	const UCommonUIActionRouterBase* ActionRouter = ULocalPlayer::GetSubsystem<UCommonUIActionRouterBase>(GetLocalPlayer());
	if (!ActionRouter || !FSlateApplication::IsInitialized())
	{
		return;
	}

	// Фокус ушёл в никуда (окно игры в фоне): оставляем то, что CommonUI запомнил сам,
	// иначе после возврата в окно ему было бы не на что вернуть фокус
	if (!FSlateApplication::Get().GetUserFocusedWidget(ActionRouter->GetLocalPlayerIndex()).IsValid())
	{
		return;
	}

	// CommonUI запоминает виджет только внутри экрана, который сейчас принимает ввод.
	// Фокус на вьюпорте игры или на экране другого слоя он пропустит сам
	ActionRouter->RefreshActiveRootFocusRestorationTarget();
}

bool URelayFocusSubsystem::HasPointerMovedThisFrame() const
{
	const UCommonUIActionRouterBase* ActionRouter = ULocalPlayer::GetSubsystem<UCommonUIActionRouterBase>(GetLocalPlayer());
	return PointerMotionTracker && ActionRouter && PointerMotionTracker->HasMovedThisFrame(ActionRouter->GetLocalPlayerIndex());
}

bool URelayFocusSubsystem::IsInActiveScreen(const UWidget& Widget) const
{
	const UCommonUIActionRouterBase* ActionRouter = ULocalPlayer::GetSubsystem<UCommonUIActionRouterBase>(GetLocalPlayer());
	return ActionRouter && ActionRouter->IsWidgetInLeafmostNodeHierarchy(Widget);
}

void URelayFocusSubsystem::KeepFocusInActiveScreen() const
{
	UCommonUIActionRouterBase* ActionRouter = ULocalPlayer::GetSubsystem<UCommonUIActionRouterBase>(GetLocalPlayer());
	if (!ActionRouter || !FSlateApplication::IsInitialized())
	{
		return;
	}

	// Фокус на вьюпорте игры или вне экранов CommonUI нас не касается
	const TSharedPtr<SWidget> FocusedWidget = FSlateApplication::Get().GetUserFocusedWidget(ActionRouter->GetLocalPlayerIndex());
	UCommonActivatableWidget* FocusedScreen = FocusedWidget ? UCommonUIActionRouterBase::FindActivatable(FocusedWidget, GetLocalPlayer()) : nullptr;
	UCommonActivatableWidget* ActiveScreen = ActionRouter->GetLeafmostActivatableWidget();
	if (!FocusedScreen || !ActiveScreen || FocusedScreen == ActiveScreen)
	{
		return;
	}

	// Фокус оказался на экране, который сейчас не принимает ввод, например на магазине под диалогом.
	// Так бывает, когда отложенная передача фокуса запланирована до открытия диалога, а выполнилась после:
	// дальше стрелки водили фокус по магазину, а A открывала второй диалог. Строки магазина это уже проверяют сами,
	// здесь подстраховка для остальных виджетов. Возвращаем фокус активному экрану
	UE_LOG(LogRelayUI, Log, TEXT("Focus moved to %s, which does not take input now. Returning it to %s."), *FocusedScreen->GetName(), *ActiveScreen->GetName());
	ActionRouter->RefreshActiveRootFocus();
}

void URelayFocusSubsystem::HandleSlateFocusChanging(const FFocusEvent& FocusEvent, const FWeakWidgetPath& OldFocusedWidgetPath, const TSharedPtr<SWidget>& OldFocusedWidget, const FWidgetPath& NewFocusedWidgetPath, const TSharedPtr<SWidget>& NewFocusedWidget)
{
	if (!NewFocusedWidget.IsValid() || RefreshTicker.IsValid())
	{
		return;
	}

	const UCommonUIActionRouterBase* ActionRouter = ULocalPlayer::GetSubsystem<UCommonUIActionRouterBase>(GetLocalPlayer());
	if (!ActionRouter || static_cast<int32>(FocusEvent.GetUser()) != ActionRouter->GetLocalPlayerIndex())
	{
		return;
	}

	// CommonUI запоминает виджет в UCommonUIActionRouterBase::HandleSlateFocusChanging на этом же событии,
	// то есть до смены фокуса, а возвращает его в FActivatableTreeRoot::HandleInputMethodChanged.
	// Тик нужен, потому что фокус сменится только после события. Навигационные кнопки обновляют сразу
	// в HandleFocusReceived, тик - запасной путь для остальных виджетов
	RefreshTicker = FTSTicker::GetCoreTicker().AddTicker(FTickerDelegate::CreateWeakLambda(this, [this](float)
	{
		RefreshTicker.Reset();
		KeepFocusInActiveScreen();
		RefreshRememberedFocus();
		return false;
	}));
}
