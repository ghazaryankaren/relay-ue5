#pragma once

#include "Engine/DeveloperSettings.h"
#include "RelayShopSettings.generated.h"

class URelayShopCatalog;

/** Relay shop settings: Project Settings > Game > Relay Shop. */
UCLASS(Config = Game, DefaultConfig, meta = (DisplayName = "Relay Shop"))
class URelayShopSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	/** Catalog every wallet sells from. */
	UPROPERTY(Config, EditAnywhere, Category = "Shop")
	TSoftObjectPtr<URelayShopCatalog> Catalog;

	/** Coins a player has at the start of a session. */
	UPROPERTY(Config, EditAnywhere, Category = "Shop", meta = (ClampMin = 0))
	int32 StartingBalance = 1500;
};
