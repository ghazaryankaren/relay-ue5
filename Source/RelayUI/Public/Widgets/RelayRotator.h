#pragma once

#include "CommonRotator.h"
#include "RelayRotator.generated.h"

class UButton;

/**
 * Rotator that the mouse can turn too. Gamepad and keyboard turn it with left and right, as any CommonRotator;
 * the optional PreviousButton and NextButton turn it on click. Keep both buttons not focusable,
 * so navigation stays on the rotator.
 */
UCLASS(Abstract, Blueprintable, meta = (DisableNativeTick))
class RELAYUI_API URelayRotator : public UCommonRotator
{
	GENERATED_BODY()

protected:
	virtual void NativeOnInitialized() override;

private:
	UFUNCTION()
	void HandlePreviousClicked();

	UFUNCTION()
	void HandleNextClicked();

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UButton> PreviousButton;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UButton> NextButton;
};
