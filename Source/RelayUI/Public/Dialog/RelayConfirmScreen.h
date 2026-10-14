#pragma once

#include "Router/RelayScreen.h"
#include "RelayConfirmScreen.generated.h"

class URelayButton;
class URelayConfirmViewModel;

/**
 * Confirmation dialog for the modal layer. Its screen context must be a URelayConfirmViewModel.
 * Confirm answers true; Cancel, Back and any other way of closing answer false. Focus starts on Cancel,
 * so a double press does not confirm by accident.
 */
UCLASS(Abstract, Blueprintable, meta = (DisableNativeTick))
class RELAYUI_API URelayConfirmScreen : public URelayScreen
{
	GENERATED_BODY()

public:
	virtual void SetScreenContext(URelayViewModelBase* Context) override;

protected:
	virtual void NativeOnInitialized() override;
	virtual void NativeOnDeactivated() override;
	virtual void NativeDestruct() override;
	virtual UWidget* NativeGetDesiredFocusTarget() const override;

private:
	void HandleConfirmClicked();
	void HandleCancelClicked();
	void CloseWithAnswer(bool bConfirmed);
	void Answer();

	// BlueprintReadOnly: MVVM bindings in the widget Blueprint (SetLabel) can reach only Blueprint-visible widget properties
	/** Confirm button. A URelayButton, because its label comes from the viewmodel through SetLabel. */
	UPROPERTY(BlueprintReadOnly, Category = "Widgets", meta = (BindWidget, AllowPrivateAccess = true))
	TObjectPtr<URelayButton> ConfirmButton;

	/** Cancel button. Has focus when the dialog opens. */
	UPROPERTY(BlueprintReadOnly, Category = "Widgets", meta = (BindWidget, AllowPrivateAccess = true))
	TObjectPtr<URelayButton> CancelButton;

	UPROPERTY(Transient)
	TObjectPtr<URelayConfirmViewModel> ViewModel;

	// Answer sent when the dialog closes. False by default: Back and a close by the router mean no
	bool bAnswer = false;
};
