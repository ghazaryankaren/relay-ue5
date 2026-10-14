#include "Debug/RelayUIDebug.h"

#include "HAL/IConsoleManager.h"

FRelayUIStats& FRelayUIStats::Get()
{
	static FRelayUIStats Stats;
	return Stats;
}

void FRelayUIStats::Reset()
{
	*this = FRelayUIStats();
}

namespace RelayUIDebug
{
	static int32 GShowStats = 0;
	static int32 GListInvalidation = 1;
	static float GIconLoadDelay = 0.f;

	FSimpleMulticastDelegate& OnStatsOverlayVisibilityChanged()
	{
		static FSimpleMulticastDelegate Delegate;
		return Delegate;
	}

	FSimpleMulticastDelegate& OnListInvalidationChanged()
	{
		static FSimpleMulticastDelegate Delegate;
		return Delegate;
	}

	bool IsStatsOverlayVisible()
	{
		return GShowStats != 0;
	}

	bool IsListInvalidationEnabled()
	{
		return GListInvalidation != 0;
	}

	float GetIconLoadDelay()
	{
		return FMath::Max(0.f, GIconLoadDelay);
	}

	// Виджеты не опрашивают переменные каждый кадр: при изменении рассылается событие
	static FAutoConsoleVariableRef CVarShowStats(
		TEXT("Relay.UI.ShowStats"),
		GShowStats,
		TEXT("1 shows the Relay UI stats overlay: live widgets, list entry reuse, icon loads, screen open time."),
		FConsoleVariableDelegate::CreateLambda([](IConsoleVariable*) { OnStatsOverlayVisibilityChanged().Broadcast(); }));

	static FAutoConsoleVariableRef CVarListInvalidation(
		TEXT("Relay.UI.ListInvalidation"),
		GListInvalidation,
		TEXT("1 lets the InvalidationBox around the shop list cache its widgets, 0 repaints them every frame. For before and after measurements."),
		FConsoleVariableDelegate::CreateLambda([](IConsoleVariable*) { OnListInvalidationChanged().Broadcast(); }));

	static FAutoConsoleVariableRef CVarIconLoadDelay(
		TEXT("Relay.UI.IconLoadDelay"),
		GIconLoadDelay,
		TEXT("Seconds to hold a loaded icon before showing it. Local loads take milliseconds; a delay makes placeholders, cancellation and reuse races visible."));

	static FAutoConsoleCommand ResetStatsCommand(
		TEXT("Relay.UI.ResetStats"),
		TEXT("Resets the counters of the Relay UI stats overlay."),
		FConsoleCommandDelegate::CreateLambda([]() { FRelayUIStats::Get().Reset(); }));
}
