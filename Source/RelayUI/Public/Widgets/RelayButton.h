#pragma once

#include "CommonButtonBase.h"
#include "RelayButton.generated.h"

class UCommonTextBlock;

/**
 * Base button of Relay. Shows the optional FocusFrame widget while it has keyboard or gamepad focus,
 * and takes focus when the mouse really moves over it, so the highlighted button is the one Enter presses.
 */
UCLASS(Abstract, Blueprintable, meta = (DisableNativeTick))
class RELAYUI_API URelayButton : public UCommonButtonBase
{
	GENERATED_BODY()

public:
	/** Sets the text of the optional LabelText widget. */
	UFUNCTION(BlueprintCallable, Category = "Button")
	void SetLabel(FText InLabel);

protected:
	virtual void NativePreConstruct() override;
	virtual void NativeConstruct() override;
	virtual void NativeOnHovered() override;
	virtual void HandleFocusReceived() override;
	virtual void HandleFocusLost() override;
	virtual void OnInputMethodChanged(ECommonInputType CurrentInputType) override;

	/** Button text, shown in the optional LabelText widget. */
	UPROPERTY(EditAnywhere, Category = "Button")
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
