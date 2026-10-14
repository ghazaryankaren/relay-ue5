#include "Settings/RelayGameUserSettings.h"

#include "AudioDevice.h"
#include "AudioDeviceManager.h"
#include "Engine/Engine.h"

URelayGameUserSettings* URelayGameUserSettings::Get()
{
	return GEngine ? Cast<URelayGameUserSettings>(GEngine->GetGameUserSettings()) : nullptr;
}

void URelayGameUserSettings::SetMasterVolume(float NewVolume)
{
	MasterVolume = FMath::Clamp(NewVolume, 0.f, 1.f);
}

void URelayGameUserSettings::SetToDefaults()
{
	Super::SetToDefaults();
	MasterVolume = 1.f;
}

void URelayGameUserSettings::ApplyNonResolutionSettings()
{
	Super::ApplyNonResolutionSettings();
	ApplyMasterVolume();
}

void URelayGameUserSettings::PostInitProperties()
{
	Super::PostInitProperties();

	// Новое аудиоустройство начинает с громкостью 1. В PIE у каждого запуска своё устройство,
	// и без подписки сохранённая громкость не применялась бы до следующего Apply
	if (!HasAnyFlags(RF_ClassDefaultObject))
	{
		AudioDeviceCreatedHandle = FAudioDeviceManagerDelegates::OnAudioDeviceCreated.AddUObject(this, &ThisClass::HandleAudioDeviceCreated);
	}
}

void URelayGameUserSettings::BeginDestroy()
{
	FAudioDeviceManagerDelegates::OnAudioDeviceCreated.Remove(AudioDeviceCreatedHandle);
	AudioDeviceCreatedHandle.Reset();

	Super::BeginDestroy();
}

void URelayGameUserSettings::ApplyMasterVolume() const
{
	// Общая громкость аудиоустройств, без SoundMix и SoundClass: своих звуковых ассетов у проекта нет.
	// Именно на всех устройствах: в PIE мир играет через своё устройство, а не через основное устройство редактора.
	// Значение уходит в аудиопоток командой, вызывать можно из игрового потока
	FAudioDeviceManager* Manager = GEngine ? GEngine->GetAudioDeviceManager() : nullptr;
	if (!Manager)
	{
		return;
	}

	Manager->IterateOverAllDevices([Volume = MasterVolume](Audio::FDeviceId, FAudioDevice* Device)
	{
		Device->SetTransientPrimaryVolume(Volume);
	});
}

void URelayGameUserSettings::HandleAudioDeviceCreated(uint32 DeviceId) const
{
	FAudioDeviceManager* Manager = GEngine ? GEngine->GetAudioDeviceManager() : nullptr;
	if (FAudioDevice* Device = Manager ? Manager->GetAudioDeviceRaw(DeviceId) : nullptr)
	{
		Device->SetTransientPrimaryVolume(MasterVolume);
	}
}
