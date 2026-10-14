#include "RelayUITags.h"

namespace RelayUITags
{
	const FGameplayTag& LayerHUD()
	{
		static const FGameplayTag Tag = FGameplayTag::RequestGameplayTag(TEXT("UI.Layer.HUD"));
		return Tag;
	}

	const FGameplayTag& LayerMenu()
	{
		static const FGameplayTag Tag = FGameplayTag::RequestGameplayTag(TEXT("UI.Layer.Menu"));
		return Tag;
	}

	const FGameplayTag& LayerModal()
	{
		static const FGameplayTag Tag = FGameplayTag::RequestGameplayTag(TEXT("UI.Layer.Modal"));
		return Tag;
	}

	const FGameplayTag& ScreenConfirm()
	{
		static const FGameplayTag Tag = FGameplayTag::RequestGameplayTag(TEXT("UI.Screen.Confirm"));
		return Tag;
	}

	const TArray<FGameplayTag>& AllLayers()
	{
		static const TArray<FGameplayTag> Layers = { LayerHUD(), LayerMenu(), LayerModal() };
		return Layers;
	}

	const FGameplayTag& InputModeGame()
	{
		static const FGameplayTag Tag = FGameplayTag::RequestGameplayTag(TEXT("InputMode.Game"));
		return Tag;
	}

	const FGameplayTag& InputModeMenu()
	{
		static const FGameplayTag Tag = FGameplayTag::RequestGameplayTag(TEXT("InputMode.Menu"));
		return Tag;
	}
}
