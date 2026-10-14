#pragma once

#include "ViewModel/RelayViewModelBase.h"
#include "RelayShopItemViewModel.generated.h"

class UTexture2D;
struct FRelayShopItem;

/** One shop item as the UI shows it. URelayShopViewModel creates one per catalog item and keeps them for the whole session. */
UCLASS()
class RELAYUI_API URelayShopItemViewModel : public URelayViewModelBase
{
	GENERATED_BODY()

public:
	/** Copies the item data from the catalog. Called once. */
	void Initialize(const FRelayShopItem& Item);

	/** Updates the owned and affordable state from the wallet. */
	void UpdateState(bool bInOwned, int32 Balance);

	FName GetItemId() const { return ItemId; }
	FName GetCategoryId() const { return CategoryId; }
	const FText& GetDisplayName() const { return DisplayName; }
	const FText& GetPriceText() const { return PriceText; }
	const TSoftObjectPtr<UTexture2D>& GetIcon() const { return Icon; }
	bool IsOwned() const { return bOwned; }
	bool CanBuy() const { return bCanBuy; }

private:
	UPROPERTY(BlueprintReadOnly, FieldNotify, Category = "Shop", meta = (AllowPrivateAccess = true))
	FText DisplayName;

	UPROPERTY(BlueprintReadOnly, FieldNotify, Category = "Shop", meta = (AllowPrivateAccess = true))
	FText Description;

	/** Price formatted for the current culture. */
	UPROPERTY(BlueprintReadOnly, FieldNotify, Category = "Shop", meta = (AllowPrivateAccess = true))
	FText PriceText;

	/** "Owned", "Need N more" or empty. */
	UPROPERTY(BlueprintReadOnly, FieldNotify, Category = "Shop", meta = (AllowPrivateAccess = true))
	FText StatusText;

	UPROPERTY(BlueprintReadOnly, FieldNotify, Category = "Shop", meta = (AllowPrivateAccess = true))
	bool bOwned = false;

	/** Not owned and the balance covers the price. */
	UPROPERTY(BlueprintReadOnly, FieldNotify, Category = "Shop", meta = (AllowPrivateAccess = true))
	bool bCanBuy = false;

	FName ItemId;
	FName CategoryId;
	int32 Price = 0;
	TSoftObjectPtr<UTexture2D> Icon;
};
