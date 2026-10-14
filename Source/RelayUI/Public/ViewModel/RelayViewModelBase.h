#pragma once

#include "MVVMViewModelBase.h"
#include "RelayViewModelBase.generated.h"

class UUserWidget;

/** Base class of all Relay viewmodels. Widgets receive them from C++ (Manual or Resolver) and never create them; the widget compiler enforces this. */
UCLASS(Abstract, meta = (MVVMAllowedContextCreationType = "Manual|Resolver"))
class RELAYUI_API URelayViewModelBase : public UMVVMViewModelBase
{
	GENERATED_BODY()

public:
	/**
	 * Hands this viewmodel to the widget's MVVM view: the widget's viewmodel of this class (creation type Manual) receives it,
	 * and its bindings run again. Returns false and logs a warning if the widget declares no such viewmodel.
	 */
	bool AssignTo(UUserWidget& Widget);
};
