#pragma once

#include "ViewModel/RelayViewModelBase.h"
#include "RelayShopViewModel.generated.h"

class APlayerController;
class ULocalPlayer;
class URelayShopCatalog;
class URelayShopItemViewModel;
class URelayWalletComponent;

/**
 * Viewmodel of the shop screen. Reads the catalog and the wallet, which live in the game module,
 * filters items by category and turns a click on an item into a confirmation dialog and a purchase.
 * Widgets never touch the wallet or the catalog.
 */
UCLASS()
class RELAYUI_API URelayShopViewModel : public URelayViewModelBase
{
	GENERATED_BODY()

public:
	/** Connects to the player's wallet and refreshes everything. Call when the screen opens. */
	void Open(APlayerController& PlayerController);

	/** Stops listening to the wallet. Call when the screen closes. */
	void Close();

	/** Tabs: id and label. The first tab (id None) shows all items, the rest are catalog categories. */
	const TArray<TPair<FName, FText>>& GetTabs() const { return Tabs; }

	FName GetSelectedCategory() const { return SelectedCategory; }
	void SelectCategory(FName CategoryId);

	/** Items of the selected category, in catalog order. */
	const TArray<TObjectPtr<URelayShopItemViewModel>>& GetVisibleItems() const { return VisibleItems; }

	URelayShopItemViewModel* GetSelectedItem() const { return SelectedItem; }
	void SelectItem(URelayShopItemViewModel* Item);

	/** Explains in a toast why the item cannot be bought, or asks for confirmation and then buys it. */
	void RequestPurchase(URelayShopItemViewModel* Item);

private:
	void BuildItems(const URelayShopCatalog& Catalog);
	void RefreshFromWallet();
	void RebuildVisibleItems();
	void Purchase(FName ItemId);
	void ShowToast(const FText& Message) const;
	URelayShopItemViewModel* FindItem(FName ItemId) const;

	/** Balance formatted for the current culture. */
	UPROPERTY(BlueprintReadOnly, FieldNotify, Category = "Shop", meta = (AllowPrivateAccess = true))
	FText BalanceText;

	/** Item shown in the details panel. */
	UPROPERTY(BlueprintReadOnly, FieldNotify, Category = "Shop", meta = (AllowPrivateAccess = true))
	TObjectPtr<URelayShopItemViewModel> SelectedItem;

	/** Items of the selected category. The screen puts them into its list view. */
	UPROPERTY(BlueprintReadOnly, FieldNotify, Category = "Shop", meta = (AllowPrivateAccess = true))
	TArray<TObjectPtr<URelayShopItemViewModel>> VisibleItems;

	UPROPERTY(Transient)
	TArray<TObjectPtr<URelayShopItemViewModel>> AllItems;

	TWeakObjectPtr<const URelayShopCatalog> BuiltCatalog;
	TWeakObjectPtr<URelayWalletComponent> Wallet;
	TWeakObjectPtr<ULocalPlayer> LocalPlayer;
	FDelegateHandle WalletChangedHandle;
	TArray<TPair<FName, FText>> Tabs;
	FName SelectedCategory;
};
