#pragma once

#include "ViewModel/RelayViewModelBase.h"
#include "RelaySettingsViewModel.generated.h"

/**
 * Viewmodel of the settings screen. Holds edited values apart from the saved ones:
 * nothing changes in the game until Apply, and Discard returns to the saved values.
 * Saved values live in URelayGameUserSettings (GameUserSettings.ini) and in the culture setting of the engine.
 */
UCLASS()
class RELAYUI_API URelaySettingsViewModel : public URelayViewModelBase
{
	GENERATED_BODY()

public:
	/** Reads the saved values. Call when the screen opens. */
	void Load();

	/** Applies and saves the edited values. */
	void Apply();

	/** Drops the edits and returns to the saved values. */
	void Discard();

	bool HasChanges() const { return bHasChanges; }

	void SetQualityLevel(int32 Level);
	void SetResolutionScale(float Scale);
	void SetVSync(bool bEnabled);
	void SetMasterVolume(float Volume);
	void SetLanguageIndex(int32 Index);

	/** Labels of the quality levels, index = level. */
	static TArray<FText> GetQualityLabels();

	/** Native names of the supported languages, in the order of URelayUISettings::SupportedCultures. */
	static TArray<FText> GetLanguageLabels();

private:
	struct FValues
	{
		int32 QualityLevel = 3;
		float ResolutionScale = 1.f;
		bool bVSync = false;
		float MasterVolume = 1.f;
		int32 LanguageIndex = 0;

		bool operator==(const FValues& Other) const;
	};

	FValues GetEdited() const;
	void SetEdited(const FValues& NewValues);
	static FValues ReadSaved();

	/** Overall quality: 0 Low, 1 Medium, 2 High, 3 Epic. */
	UPROPERTY(BlueprintReadOnly, FieldNotify, Category = "Settings", meta = (AllowPrivateAccess = true))
	int32 QualityLevel = 3;

	/** Resolution scale, 0 to 1 of the allowed range. */
	UPROPERTY(BlueprintReadOnly, FieldNotify, Category = "Settings", meta = (AllowPrivateAccess = true))
	float ResolutionScale = 1.f;

	UPROPERTY(BlueprintReadOnly, FieldNotify, Category = "Settings", meta = (AllowPrivateAccess = true))
	FText ResolutionScaleText;

	UPROPERTY(BlueprintReadOnly, FieldNotify, Category = "Settings", meta = (AllowPrivateAccess = true))
	bool bVSync = false;

	/** Master volume, 0 to 1. */
	UPROPERTY(BlueprintReadOnly, FieldNotify, Category = "Settings", meta = (AllowPrivateAccess = true))
	float MasterVolume = 1.f;

	UPROPERTY(BlueprintReadOnly, FieldNotify, Category = "Settings", meta = (AllowPrivateAccess = true))
	FText MasterVolumeText;

	/** Index in URelayUISettings::SupportedCultures. */
	UPROPERTY(BlueprintReadOnly, FieldNotify, Category = "Settings", meta = (AllowPrivateAccess = true))
	int32 LanguageIndex = 0;

	/** Edited values differ from the saved ones. */
	UPROPERTY(BlueprintReadOnly, FieldNotify, Category = "Settings", meta = (AllowPrivateAccess = true))
	bool bHasChanges = false;

	FValues Saved;

	// True while SetEdited hands values to bindings: controls echo them back through their change events
	bool bSettingValues = false;
};
