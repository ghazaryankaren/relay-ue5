#pragma once

#include "ViewModel/RelayViewModelBase.h"
#include "RelayConfirmViewModel.generated.h"

/** Answer of a confirmation dialog: true if the player confirmed. */
DECLARE_DELEGATE_OneParam(FRelayConfirmResolved, bool /*bConfirmed*/);

/**
 * Contents and answer of a confirmation dialog. The requester fills it, binds OnResolved and pushes UI.Screen.Confirm
 * with it as the screen context. The dialog knows only this viewmodel, never who opened it.
 */
UCLASS()
class RELAYUI_API URelayConfirmViewModel : public URelayViewModelBase
{
	GENERATED_BODY()

public:
	void Setup(const FText& InTitle, const FText& InMessage, const FText& InConfirmLabel, const FText& InCancelLabel);

	/** Calls OnResolved once. Later calls do nothing, so a dialog closed in several ways still answers once. */
	void Resolve(bool bConfirmed);

	bool IsResolved() const { return bResolved; }

	/** Called with the answer. Bind it weakly: the requester may be gone by the time the player answers. */
	FRelayConfirmResolved OnResolved;

private:
	UPROPERTY(BlueprintReadOnly, FieldNotify, Category = "Dialog", meta = (AllowPrivateAccess = true))
	FText Title;

	UPROPERTY(BlueprintReadOnly, FieldNotify, Category = "Dialog", meta = (AllowPrivateAccess = true))
	FText Message;

	UPROPERTY(BlueprintReadOnly, FieldNotify, Category = "Dialog", meta = (AllowPrivateAccess = true))
	FText ConfirmLabel;

	UPROPERTY(BlueprintReadOnly, FieldNotify, Category = "Dialog", meta = (AllowPrivateAccess = true))
	FText CancelLabel;

	bool bResolved = false;
};
