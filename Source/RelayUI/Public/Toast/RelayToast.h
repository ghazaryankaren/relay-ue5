#pragma once

#include "CommonUserWidget.h"
#include "RelayToast.generated.h"

class UTextBlock;

/** One toast: a short message. Reused from the toast layer's pool. */
UCLASS(Abstract, Blueprintable, meta = (DisableNativeTick))
class RELAYUI_API URelayToast : public UCommonUserWidget
{
	GENERATED_BODY()

public:
	void SetMessage(const FText& Message);

private:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> MessageText;
};
