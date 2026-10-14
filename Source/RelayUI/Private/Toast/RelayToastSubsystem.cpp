#include "Toast/RelayToastSubsystem.h"

#include "RelayUILog.h"
#include "RelayUISettings.h"
#include "Toast/RelayToast.h"
#include "Toast/RelayToastLayer.h"

void URelayToastSubsystem::ShowToast(const FText& Message)
{
	if (Message.IsEmpty())
	{
		return;
	}

	WaitingToasts.Add(Message);
	ShowWaitingToasts();

	// Ограничена только очередь ожидания: тост, для которого есть место на экране, показан выше.
	// Если что-то шлёт тосты в цикле, старые ожидающие выбрасываются, а не копятся без конца
	const int32 MaxQueued = FMath::Max(0, GetDefault<URelayUISettings>()->MaxQueuedToasts);
	while (WaitingToasts.Num() > MaxQueued)
	{
		UE_LOG(LogRelayUI, Verbose, TEXT("Toast queue is full, dropped: %s"), *WaitingToasts[0].ToString());
		WaitingToasts.RemoveAt(0);
	}
}

void URelayToastSubsystem::RegisterLayer(URelayToastLayer& Layer)
{
	ClearVisibleToasts();
	ToastLayer = &Layer;
	ShowWaitingToasts();
}

void URelayToastSubsystem::UnregisterLayer(URelayToastLayer& Layer)
{
	if (ToastLayer.Get() != &Layer)
	{
		return;
	}

	// Виджеты тостов уходят вместе со слоем. Ожидающие тосты остаются и появятся на новом лейауте
	ClearVisibleToasts();
	ToastLayer.Reset();
}

void URelayToastSubsystem::Deinitialize()
{
	ClearVisibleToasts();
	WaitingToasts.Reset();
	ToastLayer.Reset();

	Super::Deinitialize();
}

void URelayToastSubsystem::ShowWaitingToasts()
{
	URelayToastLayer* Layer = ToastLayer.Get();
	if (!Layer)
	{
		return;
	}

	const URelayUISettings* Settings = GetDefault<URelayUISettings>();
	const int32 MaxVisible = FMath::Max(1, Settings->MaxVisibleToasts);
	const float Duration = FMath::Max(0.5f, Settings->ToastDuration);

	while (VisibleToasts.Num() < MaxVisible && !WaitingToasts.IsEmpty())
	{
		const FText Message = WaitingToasts[0];
		WaitingToasts.RemoveAt(0);

		URelayToast* Toast = Layer->AddToast(Message);
		if (!Toast)
		{
			continue;
		}

		// Таймер ядра, а не мира: тост исчезает и на паузе, и время считается реальное
		FVisibleToast& Visible = VisibleToasts.AddDefaulted_GetRef();
		Visible.Widget = Toast;
		Visible.HideTimer = FTSTicker::GetCoreTicker().AddTicker(
			FTickerDelegate::CreateWeakLambda(this, [this, WeakToast = TWeakObjectPtr<URelayToast>(Toast)](float)
			{
				HideToast(WeakToast.Get());
				return false;
			}),
			Duration);
	}
}

void URelayToastSubsystem::HideToast(URelayToast* Toast)
{
	const int32 Index = VisibleToasts.IndexOfByPredicate([Toast](const FVisibleToast& Visible)
	{
		return Visible.Widget.Get() == Toast;
	});
	if (Index == INDEX_NONE)
	{
		return;
	}

	VisibleToasts.RemoveAt(Index);

	if (URelayToastLayer* Layer = ToastLayer.Get(); Layer && Toast)
	{
		Layer->RemoveToast(*Toast);
	}

	ShowWaitingToasts();
}

void URelayToastSubsystem::ClearVisibleToasts()
{
	URelayToastLayer* Layer = ToastLayer.Get();
	for (const FVisibleToast& Visible : VisibleToasts)
	{
		FTSTicker::RemoveTicker(Visible.HideTimer);
		if (Layer && Visible.Widget.IsValid())
		{
			Layer->RemoveToast(*Visible.Widget.Get());
		}
	}
	VisibleToasts.Reset();
}
