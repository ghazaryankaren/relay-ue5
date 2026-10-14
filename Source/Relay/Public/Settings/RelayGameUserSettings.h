#pragma once

#include "GameFramework/GameUserSettings.h"
#include "RelayGameUserSettings.generated.h"

/**
 * Game settings saved in GameUserSettings.ini: the engine settings (quality, resolution scale, VSync) plus the master volume.
 * The engine creates it from GameUserSettingsClassName in DefaultEngine.ini.
 */
UCLASS()
class RELAY_API URelayGameUserSettings : public UGameUserSettings
{
	GENERATED_BODY()

public:
	/** The settings object of this game, or nullptr if GameUserSettingsClassName points to another class. */
	static URelayGameUserSettings* Get();

	/** Master volume from 0 to 1. Takes effect in ApplySettings or ApplyNonResolutionSettings. */
	float GetMasterVolume() const { return MasterVolume; }
	void SetMasterVolume(float NewVolume);

	virtual void SetToDefaults() override;
	virtual void ApplyNonResolutionSettings() override;
	virtual void PostInitProperties() override;
	virtual void BeginDestroy() override;

private:
	void ApplyMasterVolume() const;
	void HandleAudioDeviceCreated(uint32 DeviceId) const;

	FDelegateHandle AudioDeviceCreatedHandle;

	UPROPERTY(Config)
	float MasterVolume = 1.f;
};
