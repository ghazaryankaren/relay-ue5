#pragma once

#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"
#include "RelayScreenRegistry.generated.h"

class URelayScreen;

#if WITH_EDITOR
class FDataValidationContext;
#endif

/** What to open for a screen tag and which layer to put it on. */
USTRUCT(BlueprintType)
struct RELAYUI_API FRelayScreenEntry
{
	GENERATED_BODY()

	/** Screen widget class. Loaded when the screen is opened. */
	UPROPERTY(EditDefaultsOnly, Category = "Screen")
	TSoftClassPtr<URelayScreen> WidgetClass;

	/** Layer whose stack receives the screen. */
	UPROPERTY(EditDefaultsOnly, Category = "Screen", meta = (Categories = "UI.Layer"))
	FGameplayTag Layer;
};

/** Screen registry: links a UI.Screen.* tag to a widget class and a layer. */
UCLASS(BlueprintType)
class RELAYUI_API URelayScreenRegistry : public UDataAsset
{
	GENERATED_BODY()

public:
	/** Returns the entry for a screen tag, or nullptr if the tag is not registered. */
	const FRelayScreenEntry* FindScreen(FGameplayTag ScreenTag) const;

#if WITH_EDITOR
	/** Reports empty tags, missing or deleted widget classes and unknown layers when the asset is saved. */
	virtual EDataValidationResult IsDataValid(FDataValidationContext& Context) const override;
#endif

private:
	/** Screen tag -> widget class and layer. */
	UPROPERTY(EditDefaultsOnly, Category = "Screens", meta = (Categories = "UI.Screen", ForceInlineRow))
	TMap<FGameplayTag, FRelayScreenEntry> Screens;
};
