#include "Shop/RelayShopItemPanel.h"

#include "Shop/RelayShopItemViewModel.h"
#include "Widgets/RelayAsyncImage.h"

void URelayShopItemPanel::SetItem(URelayShopItemViewModel* Item)
{
	if (!Item)
	{
		if (Icon)
		{
			Icon->CancelLoading();
		}
		SetVisibility(ESlateVisibility::Hidden);
		return;
	}

	SetVisibility(ESlateVisibility::HitTestInvisible);
	Item->AssignTo(*this);

	if (Icon)
	{
		Icon->SetSoftTexture(Item->GetIcon());
	}
}
