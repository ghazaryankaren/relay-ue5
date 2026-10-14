#include "Toast/RelayToast.h"

#include "Components/TextBlock.h"

void URelayToast::SetMessage(const FText& Message)
{
	MessageText->SetText(Message);
}
