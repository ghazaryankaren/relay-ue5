#include "Widgets/RelayNavButton.h"

#include "CommonInputSubsystem.h"
#include "CommonTextBlock.h"
#include "Engine/GameViewportClient.h"
#include "Engine/LocalPlayer.h"
#include "Focus/RelayFocusSubsystem.h"
#include "Framework/Application/SlateUser.h"
#include "Input/CommonUIActionRouterBase.h"
#include "Router/RelayScreen.h"
#include "Router/RelayUIRouterSubsystem.h"
#include "Widgets/SViewport.h"

void URelayNavButton::NativePreConstruct()
{
	Super::NativePreConstruct();

	// PreConstruct работает и в дизайнере, поэтому текст виден в превью WBP
	if (LabelText)
	{
		LabelText->SetText(Label);
	}
}

void URelayNavButton::NativeConstruct()
{
	Super::NativeConstruct();

	// Экран из пула строится заново, а рамка могла остаться видимой с прошлого показа
	UpdateFocusFrame();
}

void URelayNavButton::NativeOnHovered()
{
	Super::NativeOnHovered();

	if (CanTakeFocusFromPointer())
	{
		SetFocus();
	}
}

void URelayNavButton::HandleFocusReceived()
{
	Super::HandleFocusReceived();
	UpdateFocusFrame();

	// Фокус уже на кнопке, поэтому запоминаем её сразу, без тика подсистемы:
	// смена устройства в этом же кадре иначе вернула бы фокус на прошлую кнопку
	if (const URelayFocusSubsystem* Focus = ULocalPlayer::GetSubsystem<URelayFocusSubsystem>(GetOwningLocalPlayer()))
	{
		Focus->RefreshRememberedFocus();
	}
}

void URelayNavButton::HandleFocusLost()
{
	Super::HandleFocusLost();
	UpdateFocusFrame();
}

void URelayNavButton::OnInputMethodChanged(ECommonInputType CurrentInputType)
{
	Super::OnInputMethodChanged(CurrentInputType);
	UpdateFocusFrame();
}

void URelayNavButton::UpdateFocusFrame()
{
	if (!FocusFrame)
	{
		return;
	}

	// Состояние читаем из Slate, а не из события: обработчики в Super могут успеть ещё раз сменить фокус.
	// На таче Slate фокусирует нажатую кнопку, рамка там не нужна
	const UCommonInputSubsystem* InputSubsystem = UCommonInputSubsystem::Get(GetOwningLocalPlayer());
	const bool bTouch = InputSubsystem && InputSubsystem->GetCurrentInputType() == ECommonInputType::Touch;
	const bool bShown = !bTouch && (HasAnyUserFocus() || HasFocusedDescendants());

	// Рамка лежит поверх кнопки и не должна перехватывать мышь.
	// Hidden, а не Collapsed: при появлении рамка не меняет размер кнопки
	FocusFrame->SetVisibility(bShown ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Hidden);
}

bool URelayNavButton::CanTakeFocusFromPointer() const
{
	// Кнопка, которая выделяется фокусом (вкладка), от наведения выделяться не должна
	if (!GetIsEnabled() || !GetIsFocusable() || GetShouldSelectUponReceivingFocus() || HasAnyUserFocus() || HasFocusedDescendants())
	{
		return false;
	}

	ULocalPlayer* LocalPlayer = GetOwningLocalPlayer();
	const UCommonInputSubsystem* InputSubsystem = UCommonInputSubsystem::Get(LocalPlayer);
	if (!LocalPlayer || !InputSubsystem || InputSubsystem->GetCurrentInputType() != ECommonInputType::MouseAndKeyboard)
	{
		return false;
	}

	// Наведение без движения мыши не в счёт, см. FRelayPointerMotionTracker
	const URelayFocusSubsystem* Focus = ULocalPlayer::GetSubsystem<URelayFocusSubsystem>(LocalPlayer);
	if (!Focus || !Focus->HasPointerMovedThisFrame())
	{
		return false;
	}

	// Кнопка экрана, который сейчас не принимает ввод (например, меню под модалкой), фокус не забирает
	const UCommonUIActionRouterBase* ActionRouter = UCommonUIActionRouterBase::Get(*this);
	if (!ActionRouter || !ActionRouter->IsWidgetInLeafmostNodeHierarchy(*this))
	{
		return false;
	}

	// Как и CommonUI, не трогаем фокус, пока окно игры не в фокусе: иначе в PIE мышь над игрой
	// забирала бы клавиатуру у редактора или у другого PIE-клиента
	const TSharedPtr<FSlateUser> SlateUser = LocalPlayer->GetSlateUser();
	const TSharedPtr<SViewport> GameViewport = LocalPlayer->ViewportClient ? LocalPlayer->ViewportClient->GetGameViewportWidget() : nullptr;
	return SlateUser && GameViewport && SlateUser->IsWidgetInFocusPath(GameViewport);
}

void URelayNavButton::NativeOnClicked()
{
	Super::NativeOnClicked();

	// Заблокированная кнопка CommonUI видна и нажимается, но её действие не выполняется
	if (GetLocked())
	{
		return;
	}

	URelayUIRouterSubsystem* Router = ULocalPlayer::GetSubsystem<URelayUIRouterSubsystem>(GetOwningLocalPlayer());
	if (!Router)
	{
		return;
	}

	switch (Action)
	{
	case ERelayNavAction::Open:
		Router->PushScreen(TargetScreen);
		break;

	case ERelayNavAction::Replace:
		Router->ReplaceScreen(TargetScreen);
		break;

	case ERelayNavAction::CloseOwner:
		if (URelayScreen* OwnerScreen = GetTypedOuter<URelayScreen>())
		{
			Router->RequestCloseScreen(*OwnerScreen);
		}
		break;
	}
}
