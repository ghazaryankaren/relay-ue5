#include "Widgets/RelayRotator.h"

#include "Components/Button.h"

void URelayRotator::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	// У CommonRotator нет реакции на клик: без стрелок мышь не смогла бы поменять значение.
	// Событие поворота от стрелок приходит с bFromNavigation = false, экран настроек это учитывает
	if (PreviousButton)
	{
		PreviousButton->OnClicked.AddUniqueDynamic(this, &ThisClass::HandlePreviousClicked);
	}
	if (NextButton)
	{
		NextButton->OnClicked.AddUniqueDynamic(this, &ThisClass::HandleNextClicked);
	}
}

void URelayRotator::HandlePreviousClicked()
{
	ShiftTextLeft();
}

void URelayRotator::HandleNextClicked()
{
	ShiftTextRight();
}
