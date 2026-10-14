#pragma once

#include "Blueprint/UserWidgetPool.h"
#include "CommonUserWidget.h"
#include "RelayToastLayer.generated.h"

class UPanelWidget;
class URelayToast;

/**
 * Toast layer of the root layout, above the modal layer. Not a stack and not activatable: toasts never take input.
 * Toast widgets come from a pool, so a stream of toasts does not create a widget per message.
 */
UCLASS(Abstract, Blueprintable, meta = (DisableNativeTick))
class RELAYUI_API URelayToastLayer : public UCommonUserWidget
{
	GENERATED_BODY()

public:
	URelayToastLayer(const FObjectInitializer& ObjectInitializer);

	/** Takes a toast widget from the pool and adds it to the panel. Null if the toast class is not set. */
	URelayToast* AddToast(const FText& Message);

	/** Removes a toast from the panel and returns it to the pool. */
	void RemoveToast(URelayToast& Toast);

	virtual void ReleaseSlateResources(bool bReleaseChildren) override;

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	/** Widget class of one toast. */
	UPROPERTY(EditAnywhere, Category = "Toasts")
	TSubclassOf<URelayToast> ToastClass;

private:
	/** Panel (usually a Vertical Box) that receives the toasts, newest at the bottom. */
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UPanelWidget> ToastBox;

	UPROPERTY(Transient)
	FUserWidgetPool ToastPool;
};
