#pragma once

#include "Blueprint/IUserObjectListEntry.h"
#include "Containers/Ticker.h"
#include "Widgets/RelayButton.h"
#include "RelayShopItemEntry.generated.h"

class URelayAsyncImage;

/**
 * Row of the shop list. The list view reuses rows: one row shows many items over its life,
 * so everything it shows is reset in NativeOnListItemObjectSet, the icon included.
 * A row is a button: Enter, gamepad A and a click buy, and focus selects the item for the details panel.
 */
UCLASS(Abstract, Blueprintable, meta = (DisableNativeTick))
class RELAYUI_API URelayShopItemEntry : public URelayButton, public IUserObjectListEntry
{
	GENERATED_BODY()

protected:
	virtual void NativeOnInitialized() override;
	virtual void NativeOnListItemObjectSet(UObject* ListItemObject) override;
	virtual void NativeOnEntryReleased() override;
	virtual void NativeOnItemSelectionChanged(bool bIsSelected) override;
	virtual void HandleFocusReceived() override;

private:
	void TakeFocusFromList();

	/** Optional item icon. */
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<URelayAsyncImage> Icon;

	bool bHasShownItem = false;
	FTSTicker::FDelegateHandle FocusTicker;
};
