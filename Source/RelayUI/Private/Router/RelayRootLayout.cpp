#include "Router/RelayRootLayout.h"

#include "RelayUITags.h"
#include "Widgets/CommonActivatableWidgetContainer.h"

void URelayRootLayout::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	RegisterLayer(RelayUITags::LayerHUD(), HUDStack);
	RegisterLayer(RelayUITags::LayerMenu(), MenuStack);
	RegisterLayer(RelayUITags::LayerModal(), ModalStack);

	// Новый слой добавляется и сюда, и в RelayUITags::AllLayers. Проверка ловит расхождение при первом же запуске
	ensureMsgf(Layers.Num() == RelayUITags::AllLayers().Num(), TEXT("Root layout layers do not match RelayUITags::AllLayers()."));
}

UCommonActivatableWidgetContainerBase* URelayRootLayout::FindLayer(FGameplayTag LayerTag) const
{
	const TObjectPtr<UCommonActivatableWidgetContainerBase>* Layer = Layers.Find(LayerTag);
	return Layer ? Layer->Get() : nullptr;
}

FGameplayTag URelayRootLayout::FindLayerTagOf(const UCommonActivatableWidget& Widget) const
{
	for (const TPair<FGameplayTag, TObjectPtr<UCommonActivatableWidgetContainerBase>>& Pair : Layers)
	{
		if (Pair.Value && Pair.Value->GetWidgetList().Contains(&Widget))
		{
			return Pair.Key;
		}
	}

	return FGameplayTag();
}

void URelayRootLayout::RegisterLayer(FGameplayTag LayerTag, UCommonActivatableWidgetContainerBase* Layer)
{
	if (!ensure(Layer))
	{
		return;
	}

	// Переходы выключены: во время анимации стек отдаёт уходящий экран как активный,
	// а роутеру нужно точно знать, какой экран сейчас верхний
	Layer->SetTransitionDuration(0.f);
	Layers.Add(LayerTag, Layer);
}
