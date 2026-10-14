#include "Dialog/RelayConfirmScreen.h"

#include "Dialog/RelayConfirmViewModel.h"
#include "RelayUILog.h"
#include "Router/RelayUIRouterSubsystem.h"
#include "Widgets/RelayButton.h"

void URelayConfirmScreen::SetScreenContext(URelayViewModelBase* Context)
{
	Super::SetScreenContext(Context);

	// Экран из пула получает новый контекст при каждом показе, поэтому ответ сбрасываем здесь
	bAnswer = false;
	ViewModel = Cast<URelayConfirmViewModel>(Context);

	if (!ViewModel)
	{
		UE_LOG(LogRelayUI, Error, TEXT("%s opened without a URelayConfirmViewModel context: it will show empty texts and answer nobody."), *GetName());
		return;
	}

	ViewModel->AssignTo(*this);
}

void URelayConfirmScreen::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	ConfirmButton->OnClicked().AddUObject(this, &ThisClass::HandleConfirmClicked);
	CancelButton->OnClicked().AddUObject(this, &ThisClass::HandleCancelClicked);
}

void URelayConfirmScreen::NativeOnDeactivated()
{
	// Сначала Super: стек снимает диалог, и только потом ответ уходит запросившему.
	// Обработчик ответа может открыть другой экран или закрыть экран под диалогом
	Super::NativeOnDeactivated();

	// Деактивация - не всегда закрытие: новый экран на слое модалок деактивирует диалог под собой,
	// а диалог остаётся в стеке и активируется снова, когда верхний закроется. Отвечаем, только если он снят
	const URelayUIRouterSubsystem* Router = GetRouter();
	if (Router && Router->IsScreenOpen(*this))
	{
		return;
	}

	Answer();
}

void URelayConfirmScreen::NativeDestruct()
{
	// Лейаут разобрали вместе с диалогом (новый контроллер, выход из мира): ответ «нет», чтобы он не потерялся
	Answer();

	Super::NativeDestruct();
}

void URelayConfirmScreen::Answer()
{
	if (URelayConfirmViewModel* Answered = ViewModel)
	{
		ViewModel = nullptr;
		Answered->Resolve(bAnswer);
	}
}

UWidget* URelayConfirmScreen::NativeGetDesiredFocusTarget() const
{
	return CancelButton;
}

void URelayConfirmScreen::HandleConfirmClicked()
{
	CloseWithAnswer(true);
}

void URelayConfirmScreen::HandleCancelClicked()
{
	CloseWithAnswer(false);
}

void URelayConfirmScreen::CloseWithAnswer(bool bConfirmed)
{
	// Ответ отдаёт NativeOnDeactivated: туда приходят все способы закрытия, и ответ уходит ровно один раз
	bAnswer = bConfirmed;

	if (URelayUIRouterSubsystem* Router = GetRouter())
	{
		Router->RequestCloseScreen(*this);
	}

	// Роутер не смог закрыть диалог: ответ «да» не должен уйти позже через Back
	if (IsActivated())
	{
		bAnswer = false;
	}
}
