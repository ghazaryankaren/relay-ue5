#include "Game/RelayPlayerState.h"

#include "Shop/RelayWalletComponent.h"

ARelayPlayerState::ARelayPlayerState(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	Wallet = CreateDefaultSubobject<URelayWalletComponent>(TEXT("Wallet"));
}
