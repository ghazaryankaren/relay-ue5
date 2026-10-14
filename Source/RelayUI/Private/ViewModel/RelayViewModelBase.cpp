#include "ViewModel/RelayViewModelBase.h"

#include "Blueprint/UserWidget.h"
#include "MVVMSubsystem.h"
#include "RelayUILog.h"
#include "View/MVVMView.h"

bool URelayViewModelBase::AssignTo(UUserWidget& Widget)
{
	// View создаётся в NativeOnInitialized виджета и только если в WBP объявлен viewmodel или есть привязки
	UMVVMView* View = UMVVMSubsystem::GetViewFromUserWidget(&Widget);
	if (!View)
	{
		UE_LOG(LogRelayUI, Warning, TEXT("%s has no MVVM view: add a %s viewmodel with creation type Manual in the widget Blueprint."),
			*Widget.GetClass()->GetName(), *GetClass()->GetName());
		return false;
	}

	// По классу, а не по имени: имя viewmodel в WBP дизайнер может поменять, класс у экрана один
	return View->SetViewModelByClass(this);
}
