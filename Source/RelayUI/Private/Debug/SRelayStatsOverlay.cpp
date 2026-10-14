#include "Debug/SRelayStatsOverlay.h"

#include "Blueprint/UserWidget.h"
#include "Debug/RelayUIDebug.h"
#include "Engine/World.h"
#include "Misc/App.h"
#include "Styling/CoreStyle.h"
#include "UObject/UObjectIterator.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Text/STextBlock.h"

#define LOCTEXT_NAMESPACE "RelayStatsOverlay"

void SRelayStatsOverlay::Construct(const FArguments& InArgs)
{
	World = InArgs._World;

	ChildSlot
	[
		SNew(SBorder)
		.BorderImage(FCoreStyle::Get().GetBrush("GenericWhiteBox"))
		.BorderBackgroundColor(FLinearColor(0.f, 0.f, 0.f, 0.7f))
		.Padding(FMargin(8.f, 6.f))
		[
			SAssignNew(StatsText, STextBlock)
			.Font(FCoreStyle::GetDefaultFontStyle("Mono", 10))
			.ColorAndOpacity(FLinearColor::White)
		]
	];

	// Активный таймер Slate вызывается, только пока виджет на экране. Свёрнутый оверлей ничего не считает
	RegisterActiveTimer(0.25f, FWidgetActiveTimerDelegate::CreateSP(this, &SRelayStatsOverlay::Refresh));
}

EActiveTimerReturnType SRelayStatsOverlay::Refresh(double InCurrentTime, float InDeltaTime)
{
	// Сглаженное время кадра: одно значение раз в четверть секунды скачет слишком сильно
	const double FrameMs = FApp::GetDeltaTime() * 1000.0;
	SmoothedFrameMs = SmoothedFrameMs <= 0.0 ? FrameMs : FMath::Lerp(SmoothedFrameMs, FrameMs, 0.2);

	StatsText->SetText(BuildText());
	return EActiveTimerReturnType::Continue;
}

FText SRelayStatsOverlay::BuildText() const
{
	// Перебор всех UUserWidget в памяти - дорого для игры, но это отладочный оверлей и раз в четверть секунды.
	// Считаем только виджеты этого мира: в PIE рядом живут виджеты редактора
	int32 AliveWidgets = 0;
	int32 WidgetsWithSlate = 0;
	const UWorld* OverlayWorld = World.Get();
	for (TObjectIterator<UUserWidget> It(RF_ClassDefaultObject | RF_ArchetypeObject); It; ++It)
	{
		if (It->GetWorld() != OverlayWorld)
		{
			continue;
		}

		++AliveWidgets;
		if (It->GetCachedWidget().IsValid())
		{
			++WidgetsWithSlate;
		}
	}

	const FRelayUIStats& Stats = FRelayUIStats::Get();

	FString Text;
	Text += FString::Printf(TEXT("Relay UI stats            Relay.UI.ResetStats\n"));
	Text += FString::Printf(TEXT("Frame          %6.1f ms\n"), SmoothedFrameMs);
	Text += FString::Printf(TEXT("User widgets   %d alive, %d with Slate\n"), AliveWidgets, WidgetsWithSlate);
	Text += FString::Printf(TEXT("List rows      %d created, %d reused\n"), Stats.ListEntriesCreated, Stats.ListEntriesReused);
	Text += FString::Printf(TEXT("Icons          %d loads, %d done, %d cancelled, %d stale, %d in memory\n"),
		Stats.IconLoadsStarted, Stats.IconLoadsFinished, Stats.IconLoadsCancelled, Stats.IconStaleCallbacks, Stats.IconMemoryHits);
	Text += FString::Printf(TEXT("Last screen    %s: load %.1f ms, show %.1f ms\n"),
		Stats.LastScreen.IsValid() ? *Stats.LastScreen.ToString() : TEXT("-"), Stats.LastScreenLoadMs, Stats.LastScreenShowMs);
	Text += FString::Printf(TEXT("List cache     %s (Relay.UI.ListInvalidation)\n"), RelayUIDebug::IsListInvalidationEnabled() ? TEXT("on") : TEXT("off"));
	Text += FString::Printf(TEXT("Icon delay     %.2f s (Relay.UI.IconLoadDelay)"), RelayUIDebug::GetIconLoadDelay());

	// Отладочный текст не переводится
	return FText::AsCultureInvariant(Text);
}

#undef LOCTEXT_NAMESPACE
