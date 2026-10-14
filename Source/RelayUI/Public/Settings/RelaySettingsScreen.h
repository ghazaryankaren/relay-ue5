#pragma once

#include "Router/RelayScreen.h"
#include "RelaySettingsScreen.generated.h"

class UCheckBox;
class UCommonButtonBase;
class UCommonRotator;
class URelaySettingsViewModel;
class USlider;

/**
 * Settings screen. Control values come from URelaySettingsViewModel through MVVM bindings in the widget Blueprint;
 * player input goes back to the viewmodel from here. Edits apply only on Apply.
 * Leaving with unsaved edits asks for confirmation: HandleCloseRequest refuses and opens the dialog.
 */
UCLASS(Abstract, Blueprintable, meta = (DisableNativeTick))
class RELAYUI_API URelaySettingsScreen : public URelayScreen
{
	GENERATED_BODY()

public:
	virtual bool HandleCloseRequest() override;

protected:
	virtual void NativeOnInitialized() override;
	virtual void NativeOnActivated() override;
	virtual UWidget* NativeGetDesiredFocusTarget() const override;

private:
	void HandleQualityRotated(int32 Value, bool bFromNavigation);
	void HandleLanguageRotated(int32 Value, bool bFromNavigation);
	void HandleApplyClicked();
	void AskToLeaveWithoutSaving();

	UFUNCTION()
	void HandleResolutionScaleChanged(float Value);

	UFUNCTION()
	void HandleMasterVolumeChanged(float Value);

	UFUNCTION()
	void HandleVSyncChanged(bool bIsChecked);

	// BlueprintReadOnly: MVVM bindings in the widget Blueprint can reach only Blueprint-visible widget properties
	/** Use a URelayRotator Blueprint, so the mouse can turn it with its arrow buttons. */
	UPROPERTY(BlueprintReadOnly, Category = "Widgets", meta = (BindWidget, AllowPrivateAccess = true))
	TObjectPtr<UCommonRotator> QualityRotator;

	/** Use an Analog Slider and turn off Requires Controller Lock, so the gamepad changes it with left and right. */
	UPROPERTY(BlueprintReadOnly, Category = "Widgets", meta = (BindWidget, AllowPrivateAccess = true))
	TObjectPtr<USlider> ResolutionScaleSlider;

	UPROPERTY(BlueprintReadOnly, Category = "Widgets", meta = (BindWidget, AllowPrivateAccess = true))
	TObjectPtr<UCheckBox> VSyncCheckBox;

	/** Use an Analog Slider and turn off Requires Controller Lock. */
	UPROPERTY(BlueprintReadOnly, Category = "Widgets", meta = (BindWidget, AllowPrivateAccess = true))
	TObjectPtr<USlider> VolumeSlider;

	UPROPERTY(BlueprintReadOnly, Category = "Widgets", meta = (BindWidget, AllowPrivateAccess = true))
	TObjectPtr<UCommonRotator> LanguageRotator;

	UPROPERTY(BlueprintReadOnly, Category = "Widgets", meta = (BindWidget, AllowPrivateAccess = true))
	TObjectPtr<UCommonButtonBase> ApplyButton;

	UPROPERTY(Transient)
	TObjectPtr<URelaySettingsViewModel> ViewModel;
};
