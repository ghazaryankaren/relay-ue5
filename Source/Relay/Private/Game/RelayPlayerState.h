#pragma once

#include "GameFramework/PlayerState.h"
#include "RelayPlayerState.generated.h"

class URelayWalletComponent;

/** Player state with the shop wallet. */
UCLASS()
class ARelayPlayerState : public APlayerState
{
	GENERATED_BODY()

public:
	ARelayPlayerState(const FObjectInitializer& ObjectInitializer);

private:
	UPROPERTY(VisibleAnywhere, Category = "Shop")
	TObjectPtr<URelayWalletComponent> Wallet;
};
