#include "Settings/RelaySettingsScreen.h"

#include "CommonButtonBase.h"
#include "CommonRotator.h"
#include "Components/CheckBox.h"
#include "Components/Slider.h"
#include "Dialog/RelayConfirmViewModel.h"
#include "Engine/LocalPlayer.h"
#include "RelayUITags.h"
#include "Router/RelayUIRouterSubsystem.h"
#include "Settings/RelaySettingsViewModel.h"
#include "Toast/RelayToastSubsystem.h"

#define LOCTEXT_NAMESPACE "RelaySettings"

bool URelaySettingsScreen::HandleCloseRequest()
{
	if (!ViewModel->HasChanges())
	{
		return true;
	}

	// Экран не закрывается сам: спрашиваем, и при согласии закрываемся через роутер ещё раз, уже без изменений
	AskToLeaveWithoutSaving();
	return false;
}

void URelaySettingsScreen::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	// Подписи ротаторов ставим до viewmodel: привязка MVVM сразу выберет пункт, а пустой ротатор выбор не примет
	QualityRotator->PopulateTextLabels(URelaySettingsViewModel::GetQualityLabels());
	LanguageRotator->PopulateTextLabels(URelaySettingsViewModel::GetLanguageLabels());

	// Ползунки работают в долях: 0..1, шаг 5% для клавиатуры и геймпада
	for (USlider* Slider : { ResolutionScaleSlider.Get(), VolumeSlider.Get() })
	{
		Slider->SetMinValue(0.f);
		Slider->SetMaxValue(1.f);
		Slider->SetStepSize(0.05f);
	}

	ViewModel = NewObject<URelaySettingsViewModel>(this);
	ViewModel->AssignTo(*this);

	// Значения в контролы приходят привязками MVVM, а ввод игрока возвращается во viewmodel отсюда.
	// USlider::SetValue от привязки тоже вызывает OnValueChanged. Петли нет: viewmodel игнорирует
	// вызовы, пока сам раздаёт значения (SetEdited), а одинаковое значение не рассылает повторно
	QualityRotator->OnRotatedEvent.AddUObject(this, &ThisClass::HandleQualityRotated);
	LanguageRotator->OnRotatedEvent.AddUObject(this, &ThisClass::HandleLanguageRotated);
	ResolutionScaleSlider->OnValueChanged.AddDynamic(this, &ThisClass::HandleResolutionScaleChanged);
	VolumeSlider->OnValueChanged.AddDynamic(this, &ThisClass::HandleMasterVolumeChanged);
	VSyncCheckBox->OnCheckStateChanged.AddDynamic(this, &ThisClass::HandleVSyncChanged);
	ApplyButton->OnClicked().AddUObject(this, &ThisClass::HandleApplyClicked);
}

void URelaySettingsScreen::NativeOnActivated()
{
	Super::NativeOnActivated();

	// Экран из пула: правки прошлого показа уже сброшены (закрыться с ними нельзя), читаем сохранённое заново
	ViewModel->Load();
}

UWidget* URelaySettingsScreen::NativeGetDesiredFocusTarget() const
{
	return QualityRotator;
}

// Второй параметр события CommonRotator - «с навигации», а не «от игрока»: клик по стрелке мышью приходит с false.
// Программный SetSelectedItem событие не вызывает, поэтому передаём во viewmodel любой поворот
void URelaySettingsScreen::HandleQualityRotated(int32 Value, bool bFromNavigation)
{
	ViewModel->SetQualityLevel(Value);
}

void URelaySettingsScreen::HandleLanguageRotated(int32 Value, bool bFromNavigation)
{
	ViewModel->SetLanguageIndex(Value);
}

void URelaySettingsScreen::HandleResolutionScaleChanged(float Value)
{
	ViewModel->SetResolutionScale(Value);
}

void URelaySettingsScreen::HandleMasterVolumeChanged(float Value)
{
	ViewModel->SetMasterVolume(Value);
}

void URelaySettingsScreen::HandleVSyncChanged(bool bIsChecked)
{
	ViewModel->SetVSync(bIsChecked);
}

void URelaySettingsScreen::HandleApplyClicked()
{
	if (!ViewModel->HasChanges())
	{
		return;
	}

	ViewModel->Apply();

	if (URelayToastSubsystem* Toasts = ULocalPlayer::GetSubsystem<URelayToastSubsystem>(GetOwningLocalPlayer()))
	{
		Toasts->ShowToast(LOCTEXT("SettingsSaved", "Settings saved."));
	}
}

void URelaySettingsScreen::AskToLeaveWithoutSaving()
{
	URelayUIRouterSubsystem* Router = GetRouter();
	if (!Router)
	{
		return;
	}

	URelayConfirmViewModel* Confirm = NewObject<URelayConfirmViewModel>(this);
	Confirm->Setup(
		LOCTEXT("LeaveTitle", "Unsaved changes"),
		LOCTEXT("LeaveMessage", "Leave without saving? Your changes will be lost."),
		LOCTEXT("LeaveConfirm", "Leave"),
		LOCTEXT("LeaveCancel", "Stay"));

	Confirm->OnResolved.BindWeakLambda(this, [this](bool bLeave)
	{
		if (!bLeave)
		{
			return;
		}

		ViewModel->Discard();
		if (URelayUIRouterSubsystem* CurrentRouter = GetRouter())
		{
			CurrentRouter->RequestCloseScreen(*this);
		}
	});

	Router->PushScreen(RelayUITags::ScreenConfirm(), Confirm);
}

#undef LOCTEXT_NAMESPACE
