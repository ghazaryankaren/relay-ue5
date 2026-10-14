#pragma once

#include "Engine/DataAsset.h"
#include "RelayShopCatalog.generated.h"

class UTexture2D;

#if WITH_EDITOR
class FDataValidationContext;
#endif

/** A shop category. Items refer to it by Id. */
USTRUCT(BlueprintType)
struct RELAY_API FRelayShopCategory
{
	GENERATED_BODY()

	/** Stable id. Items store it, so renaming it detaches its items. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Category")
	FName Id;

	/** Name shown on the category tab. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Category")
	FText DisplayName;
};

/** A shop item. It is bought once and then owned. */
USTRUCT(BlueprintType)
struct RELAY_API FRelayShopItem
{
	GENERATED_BODY()

	/** Stable id. The wallet stores owned items by it. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item")
	FName Id;

	/** Id of the item's category. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item", meta = (GetOptions = "GetCategoryIds"))
	FName Category;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item")
	FText DisplayName;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item", meta = (MultiLine = true))
	FText Description;

	/** Price in coins. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item", meta = (ClampMin = 0))
	int32 Price = 100;

	/** Icon. The UI loads it only when the item is on screen. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item")
	TSoftObjectPtr<UTexture2D> Icon;
};

/** Shop catalog: categories and items. A designer adds an item or a category here, without code. */
UCLASS(BlueprintType)
class RELAY_API URelayShopCatalog : public UDataAsset
{
	GENERATED_BODY()

public:
	const TArray<FRelayShopCategory>& GetCategories() const { return Categories; }
	const TArray<FRelayShopItem>& GetItems() const { return Items; }

	/** Returns the item with this id, or nullptr. */
	const FRelayShopItem* FindItem(FName ItemId) const;

#if WITH_EDITOR
	/** Reports empty and duplicate ids, unknown categories, negative prices and missing icons when the asset is saved. */
	virtual EDataValidationResult IsDataValid(FDataValidationContext& Context) const override;
#endif

private:
#if WITH_EDITOR
	/** Options of the Category drop-down of an item. */
	UFUNCTION()
	TArray<FName> GetCategoryIds() const;
#endif

	/** Shop tabs, in this order. */
	UPROPERTY(EditDefaultsOnly, Category = "Catalog", meta = (TitleProperty = "Id"))
	TArray<FRelayShopCategory> Categories;

	/** Items, in the order the shop lists them. */
	UPROPERTY(EditDefaultsOnly, Category = "Catalog", meta = (TitleProperty = "Id"))
	TArray<FRelayShopItem> Items;
};
