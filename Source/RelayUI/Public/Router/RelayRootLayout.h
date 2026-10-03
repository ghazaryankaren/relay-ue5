#pragma once

#include "CommonUserWidget.h"
#include "GameplayTagContainer.h"
#include "RelayRootLayout.generated.h"

class UCommonActivatableWidget;
class UCommonActivatableWidgetContainerBase;
class UCommonActivatableWidgetStack;
class URelayUIRouterSubsystem;

/**
 * Root widget of one player's UI. Holds the layer stacks, and only the router can reach them.
 * The layout itself is not activatable, so every screen is its own CommonUI input root:
 * the topmost active screen in paint order gets input. Paint order is the order in the widget hierarchy.
 */
UCLASS(Abstract, meta = (DisableNativeTick))
class RELAYUI_API URelayRootLayout : public UCommonUserWidget
{
	GENERATED_BODY()

	friend class URelayUIRouterSubsystem;

protected:
	virtual void NativeOnInitialized() override;

private:
	UCommonActivatableWidgetContainerBase* FindLayer(FGameplayTag LayerTag) const;

	// Empty tag if the widget is on no layer
	FGameplayTag FindLayerTagOf(const UCommonActivatableWidget& Widget) const;

	void RegisterLayer(FGameplayTag LayerTag, UCommonActivatableWidgetContainerBase* Layer);

	/** Layer UI.Layer.HUD. Must be the lowest of the three in the widget hierarchy. */
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UCommonActivatableWidgetStack> HUDStack;

	/** Layer UI.Layer.Menu. Must be above HUDStack. */
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UCommonActivatableWidgetStack> MenuStack;

	/** Layer UI.Layer.Modal. Must be the top of the three. */
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UCommonActivatableWidgetStack> ModalStack;

	UPROPERTY(Transient)
	TMap<FGameplayTag, TObjectPtr<UCommonActivatableWidgetContainerBase>> Layers;
};
