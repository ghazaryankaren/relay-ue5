#pragma once

#include "CommonTabListWidgetBase.h"
#include "RelayTabList.generated.h"

class UPanelWidget;
class URelayButton;

/**
 * Row of tabs built from data at runtime. Tab buttons are placed into the TabButtonBox panel.
 * Set Next Tab and Previous Tab Enhanced Input actions (shoulder buttons, Q and E) in the widget Blueprint.
 */
UCLASS(Abstract, Blueprintable, meta = (DisableNativeTick))
class RELAYUI_API URelayTabList : public UCommonTabListWidgetBase
{
	GENERATED_BODY()

public:
	URelayTabList(const FObjectInitializer& ObjectInitializer);

	/** Replaces all tabs with Tabs (id and label) and selects SelectedId, or the first tab if SelectedId is not among them. */
	void SetTabs(TConstArrayView<TPair<FName, FText>> Tabs, FName SelectedId);

protected:
	virtual void HandleTabCreation_Implementation(FName TabNameID, UCommonButtonBase* TabButton) override;
	virtual void HandleTabRemoval_Implementation(FName TabNameID, UCommonButtonBase* TabButton) override;

	/** Button class of one tab. Its LabelText shows the tab name. */
	UPROPERTY(EditAnywhere, Category = "Tabs")
	TSubclassOf<URelayButton> TabButtonClass;

private:
	/** Panel (usually a Horizontal Box) that receives the tab buttons. */
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UPanelWidget> TabButtonBox;

	// Label of the tab being registered: RegisterTab calls HandleTabCreation from inside
	FText PendingLabel;
};
