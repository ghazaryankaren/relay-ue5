#pragma once

#include "GameplayTagContainer.h"
#include "Widgets/RelayButton.h"
#include "RelayNavButton.generated.h"

/** What a navigation button does when clicked. */
UENUM(BlueprintType)
enum class ERelayNavAction : uint8
{
	/** Open the target screen on top of its layer. */
	Open,
	/** Open the target screen in place of the top screen of its layer. */
	Replace,
	/** Close the screen this button is placed on. */
	CloseOwner
};

/** Button that navigates between screens through the router. A designer places it in a screen and picks the action and the screen tag. */
UCLASS(Abstract, Blueprintable, meta = (DisableNativeTick))
class RELAYUI_API URelayNavButton : public URelayButton
{
	GENERATED_BODY()

protected:
	virtual void NativeOnClicked() override;

	/** What the button does. */
	UPROPERTY(EditAnywhere, Category = "Navigation")
	ERelayNavAction Action = ERelayNavAction::Open;

	/** Screen to open. Not used by CloseOwner. */
	UPROPERTY(EditAnywhere, Category = "Navigation", meta = (Categories = "UI.Screen", EditCondition = "Action != ERelayNavAction::CloseOwner"))
	FGameplayTag TargetScreen;
};
