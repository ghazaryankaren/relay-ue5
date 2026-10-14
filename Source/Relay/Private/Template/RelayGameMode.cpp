// Copyright Epic Games, Inc. All Rights Reserved.

#include "Template/RelayGameMode.h"

#include "Game/RelayPlayerState.h"

ARelayGameMode::ARelayGameMode()
{
	// Кошелёк магазина живёт на PlayerState игрока
	PlayerStateClass = ARelayPlayerState::StaticClass();
}
