#pragma once

#include "Components/ActorComponent.h"
#include "RelayWalletComponent.generated.h"

class APlayerController;
class URelayShopCatalog;

/** Result of a purchase attempt. */
UENUM(BlueprintType)
enum class ERelayPurchaseResult : uint8
{
	Success,
	/** The item is already owned. */
	AlreadyOwned,
	/** The balance is lower than the price. */
	NotEnoughMoney,
	/** The catalog has no item with this id. */
	UnknownItem
};

/**
 * Coins and owned items of one player. Lives on the PlayerState: the money belongs to the player, not to the pawn or the controller.
 * The wallet checks every purchase against the catalog itself: callers pass an item id, never a price.
 * Nothing is saved between sessions.
 */
UCLASS(ClassGroup = "Relay", meta = (BlueprintSpawnableComponent))
class RELAY_API URelayWalletComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	URelayWalletComponent();

	/** Wallet of the player that owns the controller. Null while the PlayerState has not arrived (a network client) or has no wallet. */
	static URelayWalletComponent* FindForPlayer(const APlayerController* PlayerController);

	int32 GetBalance() const { return Balance; }
	bool IsOwned(FName ItemId) const { return OwnedItems.Contains(ItemId); }
	int32 GetNumOwned() const { return OwnedItems.Num(); }

	/** Catalog the wallet sells from. Null if it is not set in Project Settings > Game > Relay Shop. */
	const URelayShopCatalog* GetCatalog() const { return Catalog; }

	/** Buys an item by id. The price comes from the catalog. */
	ERelayPurchaseResult TryPurchase(FName ItemId);

	/** Adds coins (rewards, debug). Zero and negative amounts are ignored. */
	void AddMoney(int32 Amount);

	/** Starting balance and no items, as at the start of a session. */
	void ResetToStart();

	/** Broadcast after the balance or the owned items change. */
	FSimpleMulticastDelegate OnChanged;

protected:
	virtual void InitializeComponent() override;

private:
	UPROPERTY(VisibleInstanceOnly, Category = "Wallet")
	int32 Balance = 0;

	UPROPERTY(VisibleInstanceOnly, Category = "Wallet")
	TSet<FName> OwnedItems;

	UPROPERTY(Transient)
	TObjectPtr<const URelayShopCatalog> Catalog;
};
