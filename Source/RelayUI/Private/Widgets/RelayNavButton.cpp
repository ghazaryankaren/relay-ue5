#include "Widgets/RelayNavButton.h"

#include "Engine/LocalPlayer.h"
#include "Router/RelayScreen.h"
#include "Router/RelayUIRouterSubsystem.h"

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
