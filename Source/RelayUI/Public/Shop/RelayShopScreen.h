#pragma once

#include "FieldNotificationId.h"
#include "Router/RelayScreen.h"
#include "RelayShopScreen.generated.h"

class UCommonListView;
class UInvalidationBox;
class URelayShopItemPanel;
class URelayShopViewModel;
class URelayTabList;

/**
 * Shop screen: category tabs, a virtualized item list and the details panel of the selected item.
 * Values (texts, flags) reach widgets through MVVM bindings in the widget Blueprints.
 * This class wires the structure the bindings cannot: list items, tabs and selection.
 */
UCLASS(Abstract, Blueprintable, meta = (DisableNativeTick))
class RELAYUI_API URelayShopScreen : public URelayScreen
{
	GENERATED_BODY()

protected:
	virtual void NativeOnInitialized() override;
	virtual void NativeOnActivated() override;
	virtual void NativeOnDeactivated() override;
	virtual void NativeDestruct() override;
	virtual UWidget* NativeGetDesiredFocusTarget() const override;

private:
	void HandleVisibleItemsChanged(UObject* Source, UE::FieldNotification::FFieldId FieldId);
	void HandleSelectedItemChanged(UObject* Source, UE::FieldNotification::FFieldId FieldId);
	void HandleListSelectionChanged(UObject* Item);
	void HandleListItemClicked(UObject* Item);
	void ApplyListInvalidation();

	UFUNCTION()
	void HandleTabSelected(FName TabId);

	/** Item list. Its entry widget class must be a URelayShopItemEntry. */
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UCommonListView> ItemList;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<URelayTabList> CategoryTabs;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<URelayShopItemPanel> DetailsPanel;

	/** Optional Back button below the list. Down from the last item moves focus to it. */
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UWidget> BackButton;

	/** Optional InvalidationBox around the list, switched by Relay.UI.ListInvalidation for measurements. */
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UInvalidationBox> ListInvalidation;

	UPROPERTY(Transient)
	TObjectPtr<URelayShopViewModel> ViewModel;

	FDelegateHandle ListInvalidationHandle;
	bool bBuildingTabs = false;
};
