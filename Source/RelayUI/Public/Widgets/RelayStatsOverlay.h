#pragma once

#include "Components/Widget.h"
#include "RelayStatsOverlay.generated.h"

class SRelayStatsOverlay;

/**
 * UMG wrapper of the Slate debug overlay SRelayStatsOverlay. Place it on top of the root layout.
 * Hidden until Relay.UI.ShowStats 1; it never takes input.
 */
UCLASS(meta = (DisplayName = "Relay Stats Overlay"))
class RELAYUI_API URelayStatsOverlay : public UWidget
{
	GENERATED_BODY()

public:
	virtual void ReleaseSlateResources(bool bReleaseChildren) override;

#if WITH_EDITOR
	virtual const FText GetPaletteCategory() override;
#endif

protected:
	virtual TSharedRef<SWidget> RebuildWidget() override;

private:
	void UpdateVisibility();

	TSharedPtr<SRelayStatsOverlay> MyOverlay;
	FDelegateHandle VisibilityChangedHandle;
};
