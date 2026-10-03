#pragma once

#include "Engine/DeveloperSettings.h"
#include "GameplayTagContainer.h"
#include "RelayUISettings.generated.h"

class UInputMappingContext;
class URelayRootLayout;
class URelayScreenRegistry;

/** Relay UI settings: Project Settings > Game > Relay UI. */
UCLASS(Config = Game, DefaultConfig, meta = (DisplayName = "Relay UI"))
class URelayUISettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	/** Root widget with the layer stacks. The router creates it for each local player controller. */
	UPROPERTY(Config, EditAnywhere, Category = "Router")
	TSoftClassPtr<URelayRootLayout> RootLayoutClass;

	/** Screen registry: screen tag -> widget class and layer. */
	UPROPERTY(Config, EditAnywhere, Category = "Router")
	TSoftObjectPtr<URelayScreenRegistry> ScreenRegistry;

	/** Screens the router opens as soon as the player has a controller, at most one per layer. Classes load asynchronously, so screens on different layers may appear in any order. Usually the HUD and the main menu. */
	UPROPERTY(Config, EditAnywhere, Category = "Router", meta = (Categories = "UI.Screen"))
	TArray<FGameplayTag> InitialScreens;

	/** Mapping context with UI actions (Back, OpenMenu). The router adds it for every local player controller. Keep its input mode filter off. */
	UPROPERTY(Config, EditAnywhere, Category = "Input")
	TSoftObjectPtr<UInputMappingContext> UIMappingContext;

	/** Priority of the UI mapping context. Higher than game contexts, so UI keys win. */
	UPROPERTY(Config, EditAnywhere, Category = "Input")
	int32 UIMappingContextPriority = 100;
};
