#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"

// Счётчики и переключатели для отладочного оверлея (Relay.UI.ShowStats 1).
// Счётчики общие на процесс: в PIE с несколькими клиентами они складываются
struct FRelayUIStats
{
	// Строки списка магазина: сколько виджетов создано и сколько раз готовый виджет получил другой товар
	int32 ListEntriesCreated = 0;
	int32 ListEntriesReused = 0;

	// Иконки URelayAsyncImage
	int32 IconLoadsStarted = 0;
	int32 IconLoadsFinished = 0;
	int32 IconLoadsCancelled = 0;
	int32 IconStaleCallbacks = 0;
	int32 IconMemoryHits = 0;

	// Последний открытый экран: загрузка класса (от запроса до колбэка) и AddWidget (создание или пул плюс активация)
	FGameplayTag LastScreen;
	double LastScreenLoadMs = 0.0;
	double LastScreenShowMs = 0.0;

	static FRelayUIStats& Get();
	void Reset();
};

namespace RelayUIDebug
{
	/** Relay.UI.ShowStats: show the stats overlay. */
	bool IsStatsOverlayVisible();
	FSimpleMulticastDelegate& OnStatsOverlayVisibilityChanged();

	/** Relay.UI.ListInvalidation: let the shop list's InvalidationBox cache its widgets. */
	bool IsListInvalidationEnabled();
	FSimpleMulticastDelegate& OnListInvalidationChanged();

	/** Relay.UI.IconLoadDelay: extra seconds before a loaded icon is shown, to make placeholders and races visible. */
	float GetIconLoadDelay();
}
