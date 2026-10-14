#include "Shop/RelayWalletComponent.h"

#include "GameFramework/PlayerController.h"
#include "GameFramework/PlayerState.h"
#include "RelayLog.h"
#include "Shop/RelayShopCatalog.h"
#include "Shop/RelayShopSettings.h"

URelayWalletComponent::URelayWalletComponent()
{
	PrimaryComponentTick.bCanEverTick = false;

	// InitializeComponent срабатывает при создании PlayerState, то есть раньше, чем роутер строит UI
	bWantsInitializeComponent = true;
}

URelayWalletComponent* URelayWalletComponent::FindForPlayer(const APlayerController* PlayerController)
{
	// На сетевом клиенте PlayerState приходит репликацией позже контроллера и может быть ещё пустым
	const APlayerState* PlayerState = PlayerController ? PlayerController->PlayerState.Get() : nullptr;
	return PlayerState ? PlayerState->FindComponentByClass<URelayWalletComponent>() : nullptr;
}

void URelayWalletComponent::InitializeComponent()
{
	Super::InitializeComponent();

	// Каталог маленький: в нём только данные и soft-ссылки на иконки. Грузим его один раз при создании игрока,
	// а иконки грузит UI, когда товар появляется на экране
	const URelayShopSettings* Settings = GetDefault<URelayShopSettings>();
	Catalog = Settings->Catalog.LoadSynchronous();
	if (!Catalog)
	{
		UE_LOG(LogRelay, Warning, TEXT("Shop catalog is not set or failed to load: Project Settings > Game > Relay Shop."));
	}

	Balance = FMath::Max(0, Settings->StartingBalance);
}

ERelayPurchaseResult URelayWalletComponent::TryPurchase(FName ItemId)
{
	// Пока сети нет. С сетью это станет серверным RPC: клиент присылает только Id,
	// сервер проверяет то же самое и реплицирует баланс обратно
	const FRelayShopItem* Item = Catalog ? Catalog->FindItem(ItemId) : nullptr;
	if (!Item)
	{
		UE_LOG(LogRelay, Warning, TEXT("Purchase of %s: no such item in the catalog."), *ItemId.ToString());
		return ERelayPurchaseResult::UnknownItem;
	}

	if (OwnedItems.Contains(ItemId))
	{
		return ERelayPurchaseResult::AlreadyOwned;
	}

	// Цену берём из каталога, а не от UI: пока был открыт диалог, баланс мог измениться
	if (Balance < Item->Price)
	{
		return ERelayPurchaseResult::NotEnoughMoney;
	}

	Balance -= Item->Price;
	OwnedItems.Add(ItemId);
	UE_LOG(LogRelay, Log, TEXT("Bought %s for %d. Balance %d."), *ItemId.ToString(), Item->Price, Balance);

	OnChanged.Broadcast();
	return ERelayPurchaseResult::Success;
}

void URelayWalletComponent::AddMoney(int32 Amount)
{
	if (Amount <= 0)
	{
		return;
	}

	Balance = static_cast<int32>(FMath::Min<int64>(static_cast<int64>(Balance) + Amount, MAX_int32));
	OnChanged.Broadcast();
}

void URelayWalletComponent::ResetToStart()
{
	Balance = FMath::Max(0, GetDefault<URelayShopSettings>()->StartingBalance);
	OwnedItems.Reset();
	OnChanged.Broadcast();
}
