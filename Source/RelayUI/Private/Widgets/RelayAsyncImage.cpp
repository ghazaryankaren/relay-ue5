#include "Widgets/RelayAsyncImage.h"

#include "Debug/RelayUIDebug.h"
#include "Engine/AssetManager.h"
#include "Engine/StreamableManager.h"
#include "Engine/Texture2D.h"
#include "RelayUILog.h"

void URelayAsyncImage::SetSoftTexture(TSoftObjectPtr<UTexture2D> Texture)
{
	// Та же текстура уже на экране или уже грузится: ничего не трогаем, иначе мигал бы плейсхолдер
	if (Texture.ToSoftObjectPath() == CurrentTexture.ToSoftObjectPath() && (IsLoading() || bShowingTexture))
	{
		return;
	}

	// Гонка 1: строку списка переиспользовали для другого товара, пока грузилась прошлая иконка.
	// Без отмены прошлый колбэк пришёл бы позже и нарисовал чужую иконку
	CancelLoading();
	CurrentTexture = Texture;

	// Гонка 2: пустая ссылка. RequestAsyncLoad вернул бы пустой handle, и колбэк не пришёл бы никогда
	if (Texture.IsNull())
	{
		ShowPlaceholder();
		return;
	}

	FRelayUIStats& Stats = FRelayUIStats::Get();

	// Гонка 3: текстура уже в памяти. Колбэк загрузки пришёл бы только на следующем кадре,
	// и на один кадр мигнул бы плейсхолдер. Показываем сразу.
	// С отладочной задержкой текстура из памяти тоже идёт через загрузку: редактор не выгружает загруженные ассеты,
	// и в PIE после первого показа плейсхолдер и гонки было бы уже не увидеть
	UTexture2D* LoadedTexture = Texture.Get();
	if (LoadedTexture && RelayUIDebug::GetIconLoadDelay() <= 0.f)
	{
		++Stats.IconMemoryHits;
		ShowTexture(*LoadedTexture);
		return;
	}

	ShowPlaceholder();

	const uint32 RequestId = ++LastRequestId;
	PendingRequestId = RequestId;
	++Stats.IconLoadsStarted;

	TSharedPtr<FStreamableHandle> Handle = UAssetManager::GetStreamableManager().RequestAsyncLoad(
		Texture.ToSoftObjectPath(),
		FStreamableDelegate::CreateWeakLambda(this, [this, RequestId]()
		{
			OnTextureLoaded(RequestId);
		}));

	// При s.StreamableDelegateDelayFrames=0 колбэк может прийти прямо внутри RequestAsyncLoad,
	// поэтому handle сохраняем, только если этот запрос всё ещё ждёт
	if (PendingRequestId == RequestId)
	{
		if (Handle.IsValid())
		{
			LoadHandle = MoveTemp(Handle);
		}
		else
		{
			PendingRequestId = 0;
			UE_LOG(LogRelayUI, Warning, TEXT("%s: failed to start loading %s."), *GetName(), *Texture.ToString());
		}
	}
}

void URelayAsyncImage::CancelLoading()
{
	if (!IsLoading())
	{
		return;
	}

	// Только CancelHandle гасит колбэк, который уже стоит в очереди. Чтение с диска отмена не останавливает:
	// текстура догрузится и останется в памяти, пока на неё нет ссылок
	if (LoadHandle.IsValid())
	{
		LoadHandle->CancelHandle();
		LoadHandle.Reset();
	}

	if (DelayHandle.IsValid())
	{
		FTSTicker::RemoveTicker(DelayHandle);
		DelayHandle.Reset();
	}

	PendingRequestId = 0;

	// Текстура так и не показана: при повторном запросе той же текстуры загрузка начнётся заново
	CurrentTexture.Reset();
	++FRelayUIStats::Get().IconLoadsCancelled;
}

void URelayAsyncImage::ReleaseSlateResources(bool bReleaseChildren)
{
	Super::ReleaseSlateResources(bReleaseChildren);

	// Гонка 4: экран закрыли раньше, чем догрузилась иконка. Виджет больше не на экране, загрузка ему не нужна
	CancelLoading();
}

void URelayAsyncImage::OnTextureLoaded(uint32 RequestId)
{
	// Вторая линия защиты: отменённый запрос гасит CancelHandle, но если колбэк всё же пришёл, номер устарел
	if (RequestId != PendingRequestId)
	{
		++FRelayUIStats::Get().IconStaleCallbacks;
		return;
	}

	const float Delay = RelayUIDebug::GetIconLoadDelay();
	if (Delay > 0.f)
	{
		DelayHandle = FTSTicker::GetCoreTicker().AddTicker(
			FTickerDelegate::CreateWeakLambda(this, [this, RequestId](float)
			{
				DelayHandle.Reset();
				FinishLoading(RequestId);
				return false;
			}),
			Delay);
		return;
	}

	FinishLoading(RequestId);
}

void URelayAsyncImage::FinishLoading(uint32 RequestId)
{
	if (RequestId != PendingRequestId)
	{
		++FRelayUIStats::Get().IconStaleCallbacks;
		return;
	}

	PendingRequestId = 0;
	LoadHandle.Reset();
	++FRelayUIStats::Get().IconLoadsFinished;

	// Ссылку на текстуру теперь держит кисть, handle больше не нужен
	if (UTexture2D* LoadedTexture = CurrentTexture.Get())
	{
		ShowTexture(*LoadedTexture);
	}
	else
	{
		UE_LOG(LogRelayUI, Warning, TEXT("%s: failed to load %s."), *GetName(), *CurrentTexture.ToString());
		ShowPlaceholder();
	}
}

void URelayAsyncImage::ShowTexture(UTexture2D& Texture)
{
	// Размер и режим отрисовки берём из текущей кисти: их задаёт дизайнер в WBP
	FSlateBrush NewBrush = GetBrush();
	NewBrush.SetResourceObject(&Texture);
	NewBrush.TintColor = FSlateColor(FLinearColor::White);
	SetBrush(NewBrush);
	bShowingTexture = true;
}

void URelayAsyncImage::ShowPlaceholder()
{
	SetBrush(PlaceholderBrush);
	bShowingTexture = false;
}
