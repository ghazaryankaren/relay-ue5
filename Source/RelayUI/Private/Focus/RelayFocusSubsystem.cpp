#include "Focus/RelayFocusSubsystem.h"

#include "Engine/LocalPlayer.h"
#include "Focus/RelayPointerMotionTracker.h"
#include "Framework/Application/SlateApplication.h"
#include "Input/CommonUIActionRouterBase.h"

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
		RefreshRememberedFocus();
		return false;
	}));
}
