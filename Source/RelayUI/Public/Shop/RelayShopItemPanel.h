#pragma once

#include "CommonUserWidget.h"
#include "RelayShopItemPanel.generated.h"

class URelayAsyncImage;
class URelayShopItemViewModel;

/** Details panel of the selected shop item. Display only: it takes no input. */
UCLASS(Abstract, Blueprintable, meta = (DisableNativeTick))
class RELAYUI_API URelayShopItemPanel : public UCommonUserWidget
{
	GENERATED_BODY()

public:
	/** Shows an item. Null hides the panel. */
	void SetItem(URelayShopItemViewModel* Item);

private:
	/** Optional large icon. */
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<URelayAsyncImage> Icon;
};
