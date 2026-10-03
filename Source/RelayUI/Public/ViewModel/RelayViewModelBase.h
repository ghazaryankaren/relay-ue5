#pragma once

#include "MVVMViewModelBase.h"
#include "RelayViewModelBase.generated.h"

/** Base class of all Relay viewmodels. Widgets receive them from C++ (Manual or Resolver) and never create them; the widget compiler enforces this. */
UCLASS(Abstract, meta = (MVVMAllowedContextCreationType = "Manual|Resolver"))
class RELAYUI_API URelayViewModelBase : public UMVVMViewModelBase
{
	GENERATED_BODY()
};
