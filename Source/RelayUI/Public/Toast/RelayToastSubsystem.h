#pragma once

#include "Containers/Ticker.h"
#include "Subsystems/LocalPlayerSubsystem.h"
#include "RelayToastSubsystem.generated.h"

class URelayToast;
class URelayToastLayer;

/**
 * Toast queue of one local player. Shows at most MaxVisibleToasts at once on the toast layer of the root layout;
 * the rest wait in a bounded queue. Toasts are not activatable widgets, so they never take input or focus.
 * The queue outlives the layout: toasts requested while the UI is rebuilt appear on the new layout.
 */
UCLASS()
class RELAYUI_API URelayToastSubsystem : public ULocalPlayerSubsystem
{
	GENERATED_BODY()

public:
	/** Queues a toast. When the queue is full, the oldest waiting toast is dropped. */
	void ShowToast(const FText& Message);

	/** Called by the toast layer of the root layout when it is built and torn down. */
	void RegisterLayer(URelayToastLayer& Layer);
	void UnregisterLayer(URelayToastLayer& Layer);

	virtual void Deinitialize() override;

private:
	struct FVisibleToast
	{
		TWeakObjectPtr<URelayToast> Widget;
		FTSTicker::FDelegateHandle HideTimer;
	};

	void ShowWaitingToasts();
	void HideToast(URelayToast* Toast);
	void ClearVisibleToasts();

	TWeakObjectPtr<URelayToastLayer> ToastLayer;
	TArray<FText> WaitingToasts;
	TArray<FVisibleToast> VisibleToasts;
};
