#include "Widgets/RelayStatsOverlay.h"

#include "Debug/RelayUIDebug.h"
#include "Debug/SRelayStatsOverlay.h"

#define LOCTEXT_NAMESPACE "RelayStatsOverlay"

TSharedRef<SWidget> URelayStatsOverlay::RebuildWidget()
{
	MyOverlay = SNew(SRelayStatsOverlay).World(GetWorld());

	// Переменная консоли сообщает об изменении сама, оверлей не опрашивает её каждый кадр
	if (!VisibilityChangedHandle.IsValid())
	{
		VisibilityChangedHandle = RelayUIDebug::OnStatsOverlayVisibilityChanged().AddUObject(this, &ThisClass::UpdateVisibility);
	}
	UpdateVisibility();

	return MyOverlay.ToSharedRef();
}

void URelayStatsOverlay::ReleaseSlateResources(bool bReleaseChildren)
{
	Super::ReleaseSlateResources(bReleaseChildren);

	RelayUIDebug::OnStatsOverlayVisibilityChanged().Remove(VisibilityChangedHandle);
	VisibilityChangedHandle.Reset();
	MyOverlay.Reset();
}

void URelayStatsOverlay::UpdateVisibility()
{
	// В дизайнере оверлей виден всегда, чтобы его можно было расположить
	const bool bShown = IsDesignTime() || RelayUIDebug::IsStatsOverlayVisible();
	SetVisibility(bShown ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Collapsed);
}

#if WITH_EDITOR
const FText URelayStatsOverlay::GetPaletteCategory()
{
	return LOCTEXT("PaletteCategory", "Relay");
}
#endif

#undef LOCTEXT_NAMESPACE
