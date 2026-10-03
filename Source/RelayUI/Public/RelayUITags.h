#pragma once

#include "GameplayTagContainer.h"

// Layer and screen tags are listed in Config/DefaultGameplayTags.ini and found by name: UE 5.8 does not
// allow native gameplay tags in a ClientOnly module.
namespace RelayUITags
{
	RELAYUI_API const FGameplayTag& LayerHUD();
	RELAYUI_API const FGameplayTag& LayerMenu();
	RELAYUI_API const FGameplayTag& LayerModal();

	/** All layers, bottom to top. */
	RELAYUI_API const TArray<FGameplayTag>& AllLayers();

	/** Input mode tags that CommonUI puts into Enhanced Input. CommonUI declares its own copies file-static, so they cannot be reused. */
	RELAYUI_API const FGameplayTag& InputModeGame();
	RELAYUI_API const FGameplayTag& InputModeMenu();
}
