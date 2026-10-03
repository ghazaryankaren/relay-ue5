#pragma once

#include "CommonButtonBase.h"
#include "GameplayTagContainer.h"
#include "RelayNavButton.generated.h"

class UCommonTextBlock;

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

/**
 * Button that navigates between screens through the router. A designer places it in a screen and picks the action and the screen tag.
 * Shows the optional FocusFrame widget while it has focus, and takes focus when the mouse moves over it.
 */
UCLASS(Abstract, Blueprintable, meta = (DisableNativeTick))
class RELAYUI_API URelayNavButton : public UCommonButtonBase
{
	GENERATED_BODY()

protected:
	virtual void NativePreConstruct() override;
	virtual void NativeConstruct() override;
	virtual void NativeOnClicked() override;
	virtual void NativeOnHovered() override;
	virtual void HandleFocusReceived() override;
	virtual void HandleFocusLost() override;
	virtual void OnInputMethodChanged(ECommonInputType CurrentInputType) override;

	/** What the button does. */
	UPROPERTY(EditAnywhere, Category = "Navigation")
	ERelayNavAction Action = ERelayNavAction::Open;

	/** Screen to open. Not used by CloseOwner. */
	UPROPERTY(EditAnywhere, Category = "Navigation", meta = (Categories = "UI.Screen", EditCondition = "Action != ERelayNavAction::CloseOwner"))
	FGameplayTag TargetScreen;

	/** Button text, shown in the optional LabelText widget. */
	UPROPERTY(EditAnywhere, Category = "Navigation")
	FText Label;

private:
	void UpdateFocusFrame();
	bool CanTakeFocusFromPointer() const;

	/** Optional text block named LabelText in the widget Blueprint. */
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UCommonTextBlock> LabelText;

	/** Optional widget named FocusFrame in the widget Blueprint, shown while the button has focus. Keep it Hidden in the Blueprint. */
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UWidget> FocusFrame;
};
