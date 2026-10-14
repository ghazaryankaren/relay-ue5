#pragma once

#include "Widgets/SCompoundWidget.h"

class STextBlock;
class UWorld;

// Отладочный оверлей на чистом Slate: живые виджеты, переиспользование строк списка, загрузки иконок,
// время открытия последнего экрана. Обновляется таймером Slate четыре раза в секунду, а не каждый кадр
class SRelayStatsOverlay : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SRelayStatsOverlay) {}
		/** World whose user widgets are counted. */
		SLATE_ARGUMENT(TWeakObjectPtr<UWorld>, World)
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);

private:
	EActiveTimerReturnType Refresh(double InCurrentTime, float InDeltaTime);
	FText BuildText() const;

	TWeakObjectPtr<UWorld> World;
	TSharedPtr<STextBlock> StatsText;
	double LastRefreshTime = 0.0;
	double SmoothedFrameMs = 0.0;
};
