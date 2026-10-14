#include "Shop/RelayShopCatalog.h"

#if WITH_EDITOR
#include "Misc/DataValidation.h"
#include "Misc/PackageName.h"
#endif

#define LOCTEXT_NAMESPACE "RelayShopCatalog"

const FRelayShopItem* URelayShopCatalog::FindItem(FName ItemId) const
{
	// Линейный поиск: товаров десятки, а ищем только при покупке. На тысячах товаров здесь нужна карта по Id
	return Items.FindByPredicate([ItemId](const FRelayShopItem& Item)
	{
		return Item.Id == ItemId;
	});
}

#if WITH_EDITOR
TArray<FName> URelayShopCatalog::GetCategoryIds() const
{
	TArray<FName> Ids;
	Ids.Reserve(Categories.Num());
	for (const FRelayShopCategory& Category : Categories)
	{
		Ids.Add(Category.Id);
	}
	return Ids;
}

EDataValidationResult URelayShopCatalog::IsDataValid(FDataValidationContext& Context) const
{
	EDataValidationResult Result = Super::IsDataValid(Context);

	TSet<FName> CategoryIds;
	for (const FRelayShopCategory& Category : Categories)
	{
		if (Category.Id.IsNone())
		{
			Context.AddError(LOCTEXT("EmptyCategoryId", "A category has no id."));
			Result = EDataValidationResult::Invalid;
		}
		else if (CategoryIds.Contains(Category.Id))
		{
			Context.AddError(FText::Format(LOCTEXT("DuplicateCategoryId", "Category id {0} is used twice."), FText::FromName(Category.Id)));
			Result = EDataValidationResult::Invalid;
		}
		CategoryIds.Add(Category.Id);
	}

	TSet<FName> ItemIds;
	for (const FRelayShopItem& Item : Items)
	{
		const FText ItemName = FText::FromName(Item.Id);

		if (Item.Id.IsNone())
		{
			Context.AddError(LOCTEXT("EmptyItemId", "An item has no id."));
			Result = EDataValidationResult::Invalid;
		}
		else if (ItemIds.Contains(Item.Id))
		{
			// Кошелёк хранит купленное по Id: два товара с одним Id покупались бы вместе
			Context.AddError(FText::Format(LOCTEXT("DuplicateItemId", "Item id {0} is used twice."), ItemName));
			Result = EDataValidationResult::Invalid;
		}
		ItemIds.Add(Item.Id);

		if (!CategoryIds.Contains(Item.Category))
		{
			Context.AddError(FText::Format(LOCTEXT("UnknownCategory", "Item {0} has an unknown category {1}."), ItemName, FText::FromName(Item.Category)));
			Result = EDataValidationResult::Invalid;
		}

		if (Item.Price < 0)
		{
			Context.AddError(FText::Format(LOCTEXT("NegativePrice", "Item {0} has a negative price."), ItemName));
			Result = EDataValidationResult::Invalid;
		}

		// Без иконки товар покажет плейсхолдер: это не ошибка, но дизайнер скорее всего забыл
		if (Item.Icon.IsNull())
		{
			Context.AddWarning(FText::Format(LOCTEXT("NoIcon", "Item {0} has no icon."), ItemName));
		}
		else if (!FPackageName::DoesPackageExist(Item.Icon.ToSoftObjectPath().GetLongPackageName()))
		{
			Context.AddError(FText::Format(LOCTEXT("MissingIcon", "Item {0} points to a missing icon {1}."), ItemName, FText::FromString(Item.Icon.ToString())));
			Result = EDataValidationResult::Invalid;
		}
	}

	if (Result == EDataValidationResult::NotValidated)
	{
		Result = EDataValidationResult::Valid;
	}

	return Result;
}
#endif

#undef LOCTEXT_NAMESPACE
