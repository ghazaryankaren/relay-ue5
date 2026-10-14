#include "Shop/RelayShopItemViewModel.h"

#include "Engine/Texture2D.h"
#include "Shop/RelayShopCatalog.h"

#define LOCTEXT_NAMESPACE "RelayShop"

void URelayShopItemViewModel::Initialize(const FRelayShopItem& Item)
{
	ItemId = Item.Id;
	CategoryId = Item.Category;
	Price = Item.Price;

	// Viewmodel хранит только soft-ссылку. Грузить иконку или нет, решает виджет, который её показывает
	Icon = Item.Icon;

	UE_MVVM_SET_PROPERTY_VALUE(DisplayName, Item.DisplayName);
	UE_MVVM_SET_PROPERTY_VALUE(Description, Item.Description);
	UE_MVVM_SET_PROPERTY_VALUE(PriceText, FText::AsNumber(Price));
}

void URelayShopItemViewModel::UpdateState(bool bInOwned, int32 Balance)
{
	UE_MVVM_SET_PROPERTY_VALUE(bOwned, bInOwned);
	UE_MVVM_SET_PROPERTY_VALUE(bCanBuy, !bInOwned && Balance >= Price);

	FText NewStatus;
	if (bInOwned)
	{
		NewStatus = LOCTEXT("ItemOwned", "Owned");
	}
	else if (Balance < Price)
	{
		NewStatus = FText::Format(LOCTEXT("ItemNeedMore", "Need {0} more"), FText::AsNumber(Price - Balance));
	}

	// У FText нет обычного сравнения: MVVM сравнивает тексты через IdenticalTo, а новый Format всегда «другой».
	// Поэтому статус уходит в привязки при каждом изменении баланса. Это десятки строк и только при покупке
	UE_MVVM_SET_PROPERTY_VALUE(StatusText, NewStatus);
}

#undef LOCTEXT_NAMESPACE
