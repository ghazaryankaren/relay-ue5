#pragma once

#include "Components/Image.h"
#include "Containers/Ticker.h"
#include "RelayAsyncImage.generated.h"

class UTexture2D;
struct FStreamableHandle;

/**
 * Image that loads its texture from a soft reference and shows a placeholder until the texture is in memory.
 * A new texture, CancelLoading or the release of the widget cancels the load in flight,
 * so a late callback never paints the texture of an item the widget no longer shows.
 */
UCLASS(meta = (DisplayName = "Relay Async Image"))
class RELAYUI_API URelayAsyncImage : public UImage
{
	GENERATED_BODY()

public:
	/** Shows the texture, loading it first if it is not in memory. A null reference shows the placeholder. */
	UFUNCTION(BlueprintCallable, Category = "Relay Async Image")
	void SetSoftTexture(TSoftObjectPtr<UTexture2D> Texture);

	/** Cancels the load in flight, if any. A texture already shown stays. */
	void CancelLoading();

	bool IsLoading() const { return PendingRequestId != 0; }

	virtual void ReleaseSlateResources(bool bReleaseChildren) override;

protected:
	/** Brush shown while the texture loads and when there is no texture. */
	UPROPERTY(EditAnywhere, Category = "Relay Async Image")
	FSlateBrush PlaceholderBrush;

private:
	void OnTextureLoaded(uint32 RequestId);
	void FinishLoading(uint32 RequestId);
	void ShowTexture(UTexture2D& Texture);
	void ShowPlaceholder();

	TSoftObjectPtr<UTexture2D> CurrentTexture;
	TSharedPtr<FStreamableHandle> LoadHandle;
	FTSTicker::FDelegateHandle DelayHandle;
	uint32 LastRequestId = 0;
	uint32 PendingRequestId = 0;
	bool bShowingTexture = false;
};
