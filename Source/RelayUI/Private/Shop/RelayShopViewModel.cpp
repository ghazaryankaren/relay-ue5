#include "Shop/RelayShopViewModel.h"

#include "Dialog/RelayConfirmViewModel.h"
#include "Engine/LocalPlayer.h"
#include "GameFramework/PlayerController.h"
#include "RelayUILog.h"
#include "RelayUITags.h"
#include "Router/RelayUIRouterSubsystem.h"
#include "Shop/RelayShopCatalog.h"
#include "Shop/RelayShopItemViewModel.h"
#include "Shop/RelayWalletComponent.h"
#include "Toast/RelayToastSubsystem.h"

#define LOCTEXT_NAMESPACE "RelayShop"

namespace RelayShopText
{
	static FText AlreadyOwned(const FText& ItemName)
	{
		return FText::Format(LOCTEXT("AlreadyOwned", "{0} is already yours."), ItemName);
	}

	static FText NotEnoughCoins(const FText& ItemName)
	{
		return FText::Format(LOCTEXT("NotEnoughCoins", "Not enough coins for {0}."), ItemName);
	}
}

void URelayShopViewModel::Open(APlayerController& PlayerController)
{
	Close();

	LocalPlayer = PlayerController.GetLocalPlayer();

	URelayWalletComponent* NewWallet = URelayWalletComponent::FindForPlayer(&PlayerController);
	Wallet = NewWallet;

	if (NewWallet)
	{
		WalletChangedHandle = NewWallet->OnChanged.AddUObject(this, &ThisClass::RefreshFromWallet);

		// Каталог не меняется за сессию: viewmodel товаров создаются один раз, при повторном открытии только обновляются
		const URelayShopCatalog* Catalog = NewWallet->GetCatalog();
		if (Catalog && Catalog != BuiltCatalog.Get())
		{
			BuildItems(*Catalog);
		}
	}
	else
	{
		// Без сети PlayerState создаётся вместе с контроллером. На сетевом клиенте он мог ещё не прийти, тогда магазин пуст
		UE_LOG(LogRelayUI, Warning, TEXT("Shop: the player has no wallet. Check that the game mode uses ARelayPlayerState."));
	}

	RefreshFromWallet();
	RebuildVisibleItems();
}

void URelayShopViewModel::Close()
{
	if (URelayWalletComponent* WalletPtr = Wallet.Get())
	{
		WalletPtr->OnChanged.Remove(WalletChangedHandle);
	}
	WalletChangedHandle.Reset();
}

void URelayShopViewModel::SelectCategory(FName CategoryId)
{
	if (CategoryId == SelectedCategory)
	{
		return;
	}

	SelectedCategory = CategoryId;
	RebuildVisibleItems();
}

void URelayShopViewModel::SelectItem(URelayShopItemViewModel* Item)
{
	UE_MVVM_SET_PROPERTY_VALUE(SelectedItem, Item);
}

void URelayShopViewModel::RequestPurchase(URelayShopItemViewModel* Item)
{
	if (!Item)
	{
		return;
	}

	if (!Wallet.IsValid())
	{
		ShowToast(LOCTEXT("ShopUnavailable", "The shop is not available right now."));
		return;
	}

	if (Item->IsOwned())
	{
		ShowToast(RelayShopText::AlreadyOwned(Item->GetDisplayName()));
		return;
	}

	// Это подсказка UI, а не решение: при покупке кошелёк ещё раз проверит цену по каталогу и баланс
	if (!Item->CanBuy())
	{
		ShowToast(RelayShopText::NotEnoughCoins(Item->GetDisplayName()));
		return;
	}

	URelayUIRouterSubsystem* Router = ULocalPlayer::GetSubsystem<URelayUIRouterSubsystem>(LocalPlayer.Get());
	if (!Router)
	{
		return;
	}

	URelayConfirmViewModel* Confirm = NewObject<URelayConfirmViewModel>(this);
	Confirm->Setup(
		LOCTEXT("BuyTitle", "Buy item"),
		FText::Format(LOCTEXT("BuyMessage", "Buy {0} for {1} coins?"), Item->GetDisplayName(), Item->GetPriceText()),
		LOCTEXT("BuyConfirm", "Buy"),
		LOCTEXT("BuyCancel", "Cancel"));

	// Диалог не знает, кто его открыл: он только сообщает ответ. Покупаем по Id: пока диалог открыт,
	// цена и баланс могли измениться, а кошелёк проверит всё заново
	Confirm->OnResolved.BindWeakLambda(this, [this, ItemId = Item->GetItemId()](bool bConfirmed)
	{
		if (bConfirmed)
		{
			Purchase(ItemId);
		}
	});

	Router->PushScreen(RelayUITags::ScreenConfirm(), Confirm);
}

void URelayShopViewModel::BuildItems(const URelayShopCatalog& Catalog)
{
	BuiltCatalog = &Catalog;

	AllItems.Reset(Catalog.GetItems().Num());
	for (const FRelayShopItem& Item : Catalog.GetItems())
	{
		URelayShopItemViewModel* ItemViewModel = NewObject<URelayShopItemViewModel>(this);
		ItemViewModel->Initialize(Item);
		AllItems.Add(ItemViewModel);
	}

	// Первая вкладка с пустым Id показывает все товары
	Tabs.Reset();
	Tabs.Emplace(NAME_None, LOCTEXT("AllCategory", "All"));
	for (const FRelayShopCategory& Category : Catalog.GetCategories())
	{
		Tabs.Emplace(Category.Id, Category.DisplayName);
	}

	SelectedCategory = NAME_None;
	SelectItem(nullptr);
}

void URelayShopViewModel::RefreshFromWallet()
{
	const URelayWalletComponent* WalletPtr = Wallet.Get();
	const int32 Balance = WalletPtr ? WalletPtr->GetBalance() : 0;

	UE_MVVM_SET_PROPERTY_VALUE(BalanceText, WalletPtr ? FText::AsNumber(Balance) : FText::GetEmpty());

	for (URelayShopItemViewModel* Item : AllItems)
	{
		Item->UpdateState(WalletPtr && WalletPtr->IsOwned(Item->GetItemId()), Balance);
	}
}

void URelayShopViewModel::RebuildVisibleItems()
{
	TArray<TObjectPtr<URelayShopItemViewModel>> NewItems;
	for (URelayShopItemViewModel* Item : AllItems)
	{
		if (SelectedCategory.IsNone() || Item->GetCategoryId() == SelectedCategory)
		{
			NewItems.Add(Item);
		}
	}
	VisibleItems = MoveTemp(NewItems);

	// Сначала выбор, потом список: экран ставит выбор в список, только когда товар в нём уже есть.
	// Выбранный товар остаётся, если он есть в новой категории, иначе выбираем первый
	if (!SelectedItem || !VisibleItems.Contains(SelectedItem))
	{
		SelectItem(VisibleItems.IsEmpty() ? nullptr : VisibleItems[0].Get());
	}

	UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(VisibleItems);
}

void URelayShopViewModel::Purchase(FName ItemId)
{
	URelayWalletComponent* WalletPtr = Wallet.Get();
	if (!WalletPtr)
	{
		return;
	}

	const URelayShopItemViewModel* Item = FindItem(ItemId);
	const FText ItemName = Item ? Item->GetDisplayName() : FText::FromName(ItemId);

	switch (WalletPtr->TryPurchase(ItemId))
	{
	case ERelayPurchaseResult::Success:
		ShowToast(FText::Format(LOCTEXT("Bought", "Bought {0}."), ItemName));
		break;

	case ERelayPurchaseResult::AlreadyOwned:
		ShowToast(RelayShopText::AlreadyOwned(ItemName));
		break;

	case ERelayPurchaseResult::NotEnoughMoney:
		ShowToast(RelayShopText::NotEnoughCoins(ItemName));
		break;

	case ERelayPurchaseResult::UnknownItem:
		ShowToast(LOCTEXT("NoLongerSold", "This item is no longer sold."));
		break;
	}
}

void URelayShopViewModel::ShowToast(const FText& Message) const
{
	if (URelayToastSubsystem* Toasts = ULocalPlayer::GetSubsystem<URelayToastSubsystem>(LocalPlayer.Get()))
	{
		Toasts->ShowToast(Message);
	}
}

URelayShopItemViewModel* URelayShopViewModel::FindItem(FName ItemId) const
{
	const TObjectPtr<URelayShopItemViewModel>* Found = AllItems.FindByPredicate([ItemId](const TObjectPtr<URelayShopItemViewModel>& Item)
	{
		return Item->GetItemId() == ItemId;
	});
	return Found ? Found->Get() : nullptr;
}

#undef LOCTEXT_NAMESPACE
