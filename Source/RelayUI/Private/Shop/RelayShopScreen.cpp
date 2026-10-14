#include "Shop/RelayShopScreen.h"

#include "CommonListView.h"
#include "Components/InvalidationBox.h"
#include "Debug/RelayUIDebug.h"
#include "Shop/RelayShopItemPanel.h"
#include "Shop/RelayShopItemViewModel.h"
#include "Shop/RelayShopViewModel.h"
#include "Widgets/RelayTabList.h"

void URelayShopScreen::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	// Viewmodel живёт, пока жив объект экрана: стек CommonUI держит закрытый экран в пуле,
	// и при повторном открытии товары не создаются заново
	ViewModel = NewObject<URelayShopViewModel>(this);
	ViewModel->AssignTo(*this);

	// Привязки MVVM передают значения. Список, выбор и вкладки - структура, их подключаем кодом
	ViewModel->AddFieldValueChangedDelegate(URelayShopViewModel::FFieldNotificationClassDescriptor::VisibleItems,
		INotifyFieldValueChanged::FFieldValueChangedDelegate::CreateUObject(this, &ThisClass::HandleVisibleItemsChanged));
	ViewModel->AddFieldValueChangedDelegate(URelayShopViewModel::FFieldNotificationClassDescriptor::SelectedItem,
		INotifyFieldValueChanged::FFieldValueChangedDelegate::CreateUObject(this, &ThisClass::HandleSelectedItemChanged));

	// Получив фокус, SCommonListView переводит его на выбранную строку, только если включён bReturnFocusToSelection.
	// Иначе при открытии и после смены вкладки фокус оставался на самом списке, и Enter и A ничего не нажимали.
	// Сеттер UListView::SetReturnFocusToSelection не экспортирован из UMG (класс MinimalAPI), поэтому ставим
	// свойство через рефлексию. Slate списка ещё не построен и возьмёт значение при постройке
	if (FBoolProperty* ReturnFocusProperty = FindFProperty<FBoolProperty>(UListView::StaticClass(), TEXT("bReturnFocusToSelection")))
	{
		ReturnFocusProperty->SetPropertyValue_InContainer(ItemList.Get(), true);
	}

	// Вниз с последнего товара - на Back явным правилом. Геометрический поиск Slate от последней строки
	// кнопку не находил (след SlateDebugger: Hit Test Grid -> None). Внутри списка навигацию обрабатывает
	// сам список, наружу он её отдаёт только на краю, поэтому правило действует только там
	if (BackButton)
	{
		ItemList->SetNavigationRuleExplicit(EUINavigation::Down, BackButton);
	}

	ItemList->OnItemSelectionChanged().AddUObject(this, &ThisClass::HandleListSelectionChanged);
	ItemList->OnItemClicked().AddUObject(this, &ThisClass::HandleListItemClicked);
	CategoryTabs->OnTabSelected.AddDynamic(this, &ThisClass::HandleTabSelected);
}

void URelayShopScreen::NativeOnActivated()
{
	Super::NativeOnActivated();

	if (APlayerController* PlayerController = GetOwningPlayer())
	{
		ViewModel->Open(*PlayerController);
	}

	// Вкладки строим на каждое открытие: стек освобождает Slate закрытого экрана,
	// а список вкладок в NativeDestruct удаляет все вкладки.
	// Пока строим, события выбора вкладки не трогают категорию: первая вкладка выбирается сама при регистрации
	bBuildingTabs = true;
	CategoryTabs->SetTabs(ViewModel->GetTabs(), ViewModel->GetSelectedCategory());
	bBuildingTabs = false;

	// Список Open уже обновил через событие VisibleItems. Выбранный товар мог не измениться,
	// а Slate панели деталей пересобран, поэтому показываем его явно
	if (DetailsPanel)
	{
		DetailsPanel->SetItem(ViewModel->GetSelectedItem());
	}

	ApplyListInvalidation();
	if (!ListInvalidationHandle.IsValid())
	{
		ListInvalidationHandle = RelayUIDebug::OnListInvalidationChanged().AddUObject(this, &ThisClass::ApplyListInvalidation);
	}
}

void URelayShopScreen::NativeOnDeactivated()
{
	ViewModel->Close();

	Super::NativeOnDeactivated();
}

void URelayShopScreen::NativeDestruct()
{
	RelayUIDebug::OnListInvalidationChanged().Remove(ListInvalidationHandle);
	ListInvalidationHandle.Reset();

	Super::NativeDestruct();
}

UWidget* URelayShopScreen::NativeGetDesiredFocusTarget() const
{
	// Получив фокус, список переводит его на выбранную строку (bReturnFocusToSelection, см. NativeOnInitialized)
	return ItemList;
}

void URelayShopScreen::HandleVisibleItemsChanged(UObject* Source, UE::FieldNotification::FFieldId FieldId)
{
	const bool bListHadFocus = ItemList->HasAnyUserFocus() || ItemList->HasFocusedDescendants();

	// Строки списка не создаются заново: список раздаст новые товары тем же виджетам строк
	ItemList->SetListItems(ViewModel->GetVisibleItems());
	ItemList->ScrollToTop();

	if (URelayShopItemViewModel* Selected = ViewModel->GetSelectedItem())
	{
		ItemList->SetSelectedItem(Selected);
	}
	else
	{
		ItemList->ClearSelection();
	}

	// Строка с фокусом могла получить другой товар или пропасть: строки пересобираются на следующем кадре.
	// Возвращаем фокус списку, и он сам переведёт его на выбранную строку
	if (bListHadFocus)
	{
		ItemList->SetFocus();
	}
}

void URelayShopScreen::HandleSelectedItemChanged(UObject* Source, UE::FieldNotification::FFieldId FieldId)
{
	URelayShopItemViewModel* Selected = ViewModel->GetSelectedItem();

	if (DetailsPanel)
	{
		DetailsPanel->SetItem(Selected);
	}

	// Список ещё может показывать прошлую категорию: тогда выбор поставит HandleVisibleItemsChanged
	if (Selected && ItemList->GetIndexForItem(Selected) != INDEX_NONE && ItemList->GetSelectedItem() != Selected)
	{
		ItemList->SetSelectedItem(Selected);
	}
}

void URelayShopScreen::HandleListSelectionChanged(UObject* Item)
{
	// Пустой выбор приходит, пока список меняет товары. Выбор во viewmodel он не сбрасывает
	if (URelayShopItemViewModel* ItemViewModel = Cast<URelayShopItemViewModel>(Item))
	{
		ViewModel->SelectItem(ItemViewModel);
	}
}

void URelayShopScreen::HandleListItemClicked(UObject* Item)
{
	ViewModel->RequestPurchase(Cast<URelayShopItemViewModel>(Item));
}

void URelayShopScreen::HandleTabSelected(FName TabId)
{
	if (!bBuildingTabs)
	{
		ViewModel->SelectCategory(TabId);
	}
}

void URelayShopScreen::ApplyListInvalidation()
{
	if (ListInvalidation)
	{
		ListInvalidation->SetCanCache(RelayUIDebug::IsListInvalidationEnabled());
	}
}
