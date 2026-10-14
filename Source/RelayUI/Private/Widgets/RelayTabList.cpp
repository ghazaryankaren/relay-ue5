#include "Widgets/RelayTabList.h"

#include "Components/PanelWidget.h"
#include "RelayUILog.h"
#include "Widgets/RelayButton.h"

URelayTabList::URelayTabList(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	// Действия «следующая» и «предыдущая вкладка» регистрируются, как только список построен.
	// CommonUI привязывает их к ближайшему активируемому экрану, поэтому они работают, только пока экран сверху
	bAutoListenForInput = true;
}

void URelayTabList::SetTabs(TConstArrayView<TPair<FName, FText>> Tabs, FName SelectedId)
{
	RemoveAllTabs();

	if (!TabButtonClass)
	{
		UE_LOG(LogRelayUI, Error, TEXT("%s: Tab Button Class is not set."), *GetClass()->GetName());
		return;
	}

	for (const TPair<FName, FText>& Tab : Tabs)
	{
		PendingLabel = Tab.Value;
		RegisterTab(Tab.Key, TabButtonClass, nullptr);
	}
	PendingLabel = FText::GetEmpty();

	if (Tabs.IsEmpty())
	{
		return;
	}

	const bool bHasSelected = Tabs.ContainsByPredicate([SelectedId](const TPair<FName, FText>& Tab)
	{
		return Tab.Key == SelectedId;
	});
	SelectTabByID(bHasSelected ? SelectedId : Tabs[0].Key, true);
}

void URelayTabList::HandleTabCreation_Implementation(FName TabNameID, UCommonButtonBase* TabButton)
{
	Super::HandleTabCreation_Implementation(TabNameID, TabButton);

	if (!TabButton || !TabButtonBox)
	{
		return;
	}

	TabButtonBox->AddChild(TabButton);

	if (URelayButton* RelayButton = Cast<URelayButton>(TabButton))
	{
		RelayButton->SetLabel(PendingLabel);
	}
}

void URelayTabList::HandleTabRemoval_Implementation(FName TabNameID, UCommonButtonBase* TabButton)
{
	Super::HandleTabRemoval_Implementation(TabNameID, TabButton);

	if (TabButton && TabButtonBox)
	{
		TabButtonBox->RemoveChild(TabButton);
	}
}
