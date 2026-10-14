#include "Shop/RelayShopItemEntry.h"

#include "Components/ListView.h"
#include "Components/ListViewBase.h"
#include "Debug/RelayUIDebug.h"
#include "Engine/LocalPlayer.h"
#include "Focus/RelayFocusSubsystem.h"
#include "Shop/RelayShopItemViewModel.h"
#include "Widgets/RelayAsyncImage.h"

void URelayShopItemEntry::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	++FRelayUIStats::Get().ListEntriesCreated;
}

void URelayShopItemEntry::NativeOnListItemObjectSet(UObject* ListItemObject)
{
	IUserObjectListEntry::NativeOnListItemObjectSet(ListItemObject);

	// Строка уже показывала другой товар: это и есть переиспользование, ради которого список виртуальный
	if (bHasShownItem)
	{
		++FRelayUIStats::Get().ListEntriesReused;
	}
	bHasShownItem = true;

	URelayShopItemViewModel* Item = Cast<URelayShopItemViewModel>(ListItemObject);
	if (!Item)
	{
		return;
	}

	// Viewmodel ставим сами, а не через расширение MVVM для списков: оно назначает viewmodel на кадр позже,
	// и кадр строка показывала бы прошлый товар
	Item->AssignTo(*this);

	if (Icon)
	{
		Icon->SetSoftTexture(Item->GetIcon());
	}

	// Строка получила выбранный товар при прокрутке: фокус должен быть на ней, а не на строке, которая его показывала
	if (IsListItemSelected())
	{
		TakeFocusFromList();
	}
}

void URelayShopItemEntry::NativeOnItemSelectionChanged(bool bIsSelected)
{
	IUserObjectListEntry::NativeOnItemSelectionChanged(bIsSelected);

	if (bIsSelected)
	{
		TakeFocusFromList();
	}
}

void URelayShopItemEntry::TakeFocusFromList()
{
	// SListView не всегда переводит фокус на строку выбранного товара. После прокрутки за видимые строки фокус
	// оставался на строке, которую список уже отдал другому товару: Enter нажимал не тот товар, что выделен,
	// а поиск вниз с последнего товара шёл от строки, которой нет на экране, и не находил Back.
	// Поэтому строка выбранного товара забирает фокус сама. Строку могли только что выдать из пула, и в дереве
	// Slate её ещё нет, поэтому фокус ставим на следующем тике
	if (FocusTicker.IsValid())
	{
		return;
	}

	FocusTicker = FTSTicker::GetCoreTicker().AddTicker(FTickerDelegate::CreateWeakLambda(this, [this](float)
	{
		FocusTicker.Reset();

		// Только если фокус уже в списке (клавиатура, геймпад или мышь над строкой), чтобы не уводить его, например, с вкладок
		UListViewBase* ListView = GetOwningListView();
		if (!ListView || !IsListItemSelected() || HasAnyUserFocus() || HasFocusedDescendants())
		{
			return false;
		}

		// За этот тик могли нажать A, и над магазином уже открыт диалог. Без проверки строка уводила фокус
		// из диалога в магазин под ним (стик зажат вниз, затем A)
		const URelayFocusSubsystem* Focus = ULocalPlayer::GetSubsystem<URelayFocusSubsystem>(GetOwningLocalPlayer());
		if (!Focus || !Focus->IsInActiveScreen(*this))
		{
			return false;
		}

		if (ListView->HasAnyUserFocus() || ListView->HasFocusedDescendants())
		{
			SetFocus();
		}
		return false;
	}));
}

void URelayShopItemEntry::NativeOnEntryReleased()
{
	IUserObjectListEntry::NativeOnEntryReleased();

	FTSTicker::RemoveTicker(FocusTicker);
	FocusTicker.Reset();

	// Строка ушла в пул списка: иконка прошлого товара ей больше не нужна
	if (Icon)
	{
		Icon->CancelLoading();
	}
}

void URelayShopItemEntry::HandleFocusReceived()
{
	Super::HandleFocusReceived();

	// Выбран тот товар, на котором фокус: так панель деталей одинаково следует за геймпадом, клавиатурой и мышью
	// (мышь переносит фокус на строку при настоящем движении, см. URelayButton)
	if (UListView* ListView = Cast<UListView>(GetOwningListView()))
	{
		UObject* Item = GetListItem();
		if (Item && ListView->GetSelectedItem() != Item)
		{
			ListView->SetSelectedItem(Item);
		}
	}
}
