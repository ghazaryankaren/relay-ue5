#include "Dialog/RelayConfirmViewModel.h"

void URelayConfirmViewModel::Setup(const FText& InTitle, const FText& InMessage, const FText& InConfirmLabel, const FText& InCancelLabel)
{
	UE_MVVM_SET_PROPERTY_VALUE(Title, InTitle);
	UE_MVVM_SET_PROPERTY_VALUE(Message, InMessage);
	UE_MVVM_SET_PROPERTY_VALUE(ConfirmLabel, InConfirmLabel);
	UE_MVVM_SET_PROPERTY_VALUE(CancelLabel, InCancelLabel);
}

void URelayConfirmViewModel::Resolve(bool bConfirmed)
{
	if (bResolved)
	{
		return;
	}

	bResolved = true;

	// Делегат копируем и отвязываем до вызова: обработчик может открыть новый диалог или закрыть экран
	FRelayConfirmResolved Callback = MoveTemp(OnResolved);
	OnResolved.Unbind();
	Callback.ExecuteIfBound(bConfirmed);
}
