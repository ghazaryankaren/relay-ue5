#include "Settings/RelaySettingsViewModel.h"

#include "Internationalization/Culture.h"
#include "Internationalization/Internationalization.h"
#include "Internationalization/TextLocalizationManager.h"
#include "Kismet/KismetInternationalizationLibrary.h"
#include "RelayUISettings.h"
#include "Settings/RelayGameUserSettings.h"

#define LOCTEXT_NAMESPACE "RelaySettings"

namespace RelaySettingsLanguage
{
	static FString GetActiveCulture()
	{
#if WITH_EDITOR
		if (GIsEditor)
		{
			// В PIE тексты игры показываются на исходном языке, пока не включён предпросмотр локализации
			const FTextLocalizationManager& Manager = FTextLocalizationManager::Get();
			if (!Manager.IsGameLocalizationPreviewEnabled())
			{
				const TArray<FString>& Cultures = GetDefault<URelayUISettings>()->SupportedCultures;
				return Cultures.IsEmpty() ? FString() : Cultures[0];
			}
			return Manager.GetConfiguredGameLocalizationPreviewLanguage();
		}
#endif
		return FInternationalization::Get().GetCurrentLanguage()->GetName();
	}

	static void ApplyCulture(const FString& Culture)
	{
#if WITH_EDITOR
		if (GIsEditor)
		{
			// Смена культуры процесса в PIE переключила бы язык всего редактора. Меняем только предпросмотр текстов игры.
			// Выбор пишем в настройку редактора Preview Game Language: редактор выключает предпросмотр в конце PIE
			// и включает его с этим языком в начале следующего, как сохранённая культура в собранной игре
			FTextLocalizationManager& Manager = FTextLocalizationManager::Get();
			Manager.ConfigureGameLocalizationPreviewLanguage(Culture);
			Manager.EnableGameLocalizationPreview(Culture);
			return;
		}
#endif
		// Движок сам сохраняет культуру в GameUserSettings.ini и читает её при следующем запуске
		UKismetInternationalizationLibrary::SetCurrentCulture(Culture, true);
	}

	static int32 FindCultureIndex(const FString& Culture)
	{
		const TArray<FString>& Cultures = GetDefault<URelayUISettings>()->SupportedCultures;

		// "ru-RU" подходит к "ru": сначала точное совпадение, потом по языку
		int32 Index = Cultures.IndexOfByKey(Culture);
		if (Index == INDEX_NONE)
		{
			FString Language = Culture;
			Culture.Split(TEXT("-"), &Language, nullptr);
			Index = Cultures.IndexOfByKey(Language);
		}
		return FMath::Max(0, Index);
	}
}

bool URelaySettingsViewModel::FValues::operator==(const FValues& Other) const
{
	return QualityLevel == Other.QualityLevel
		&& FMath::IsNearlyEqual(ResolutionScale, Other.ResolutionScale, 0.001f)
		&& bVSync == Other.bVSync
		&& FMath::IsNearlyEqual(MasterVolume, Other.MasterVolume, 0.001f)
		&& LanguageIndex == Other.LanguageIndex;
}

TArray<FText> URelaySettingsViewModel::GetQualityLabels()
{
	return {
		LOCTEXT("QualityLow", "Low"),
		LOCTEXT("QualityMedium", "Medium"),
		LOCTEXT("QualityHigh", "High"),
		LOCTEXT("QualityEpic", "Epic")
	};
}

TArray<FText> URelaySettingsViewModel::GetLanguageLabels()
{
	TArray<FText> Labels;
	for (const FString& Code : GetDefault<URelayUISettings>()->SupportedCultures)
	{
		// Язык называем на нём самом: «English», «Русский». Так его найдёт и тот, кто не читает текущий язык
		const FCulturePtr Culture = FInternationalization::Get().GetCulture(Code);
		FString Name = Culture.IsValid() ? Culture->GetNativeName() : Code;
		if (!Name.IsEmpty())
		{
			Name = FText::FromString(Name.Left(1)).ToUpper().ToString() + Name.Mid(1);
		}
		Labels.Add(FText::AsCultureInvariant(Name));
	}
	return Labels;
}

URelaySettingsViewModel::FValues URelaySettingsViewModel::ReadSaved()
{
	FValues Values;

	if (const UGameUserSettings* Settings = UGameUserSettings::GetGameUserSettings())
	{
		// -1 значит, что качество настроено по отдельным группам. Экран показывает его как Epic,
		// а Apply выставит всем группам один уровень
		const int32 Level = Settings->GetOverallScalabilityLevel();
		Values.QualityLevel = Level < 0 ? 3 : FMath::Clamp(Level, 0, 3);
		Values.ResolutionScale = Settings->GetResolutionScaleNormalized();
		Values.bVSync = Settings->IsVSyncEnabled();
	}

	if (const URelayGameUserSettings* RelaySettings = URelayGameUserSettings::Get())
	{
		Values.MasterVolume = RelaySettings->GetMasterVolume();
	}

	Values.LanguageIndex = RelaySettingsLanguage::FindCultureIndex(RelaySettingsLanguage::GetActiveCulture());
	return Values;
}

void URelaySettingsViewModel::Load()
{
	Saved = ReadSaved();
	SetEdited(Saved);
}

void URelaySettingsViewModel::Apply()
{
	UGameUserSettings* Settings = UGameUserSettings::GetGameUserSettings();
	if (!Settings)
	{
		return;
	}

	const FValues Edited = GetEdited();

	Settings->SetOverallScalabilityLevel(Edited.QualityLevel);
	Settings->SetResolutionScaleNormalized(Edited.ResolutionScale);
	Settings->SetVSyncEnabled(Edited.bVSync);
	if (URelayGameUserSettings* RelaySettings = Cast<URelayGameUserSettings>(Settings))
	{
		RelaySettings->SetMasterVolume(Edited.MasterVolume);
	}

	// Без ApplySettings: он применяет и разрешение окна, а экран разрешение не меняет, и в PIE это задело бы окно игры.
	// Качество, масштаб разрешения, VSync и громкость применяет ApplyNonResolutionSettings
	Settings->ApplyNonResolutionSettings();
	Settings->SaveSettings();

	const TArray<FString>& Cultures = GetDefault<URelayUISettings>()->SupportedCultures;
	if (Edited.LanguageIndex != Saved.LanguageIndex && Cultures.IsValidIndex(Edited.LanguageIndex))
	{
		RelaySettingsLanguage::ApplyCulture(Cultures[Edited.LanguageIndex]);
	}

	Saved = Edited;
	UE_MVVM_SET_PROPERTY_VALUE(bHasChanges, false);
}

void URelaySettingsViewModel::Discard()
{
	SetEdited(Saved);
}

void URelaySettingsViewModel::SetQualityLevel(int32 Level)
{
	if (bSettingValues)
	{
		return;
	}

	FValues Values = GetEdited();
	Values.QualityLevel = FMath::Clamp(Level, 0, 3);
	SetEdited(Values);
}

void URelaySettingsViewModel::SetResolutionScale(float Scale)
{
	if (bSettingValues)
	{
		return;
	}

	FValues Values = GetEdited();
	Values.ResolutionScale = FMath::Clamp(Scale, 0.f, 1.f);
	SetEdited(Values);
}

void URelaySettingsViewModel::SetVSync(bool bEnabled)
{
	if (bSettingValues)
	{
		return;
	}

	FValues Values = GetEdited();
	Values.bVSync = bEnabled;
	SetEdited(Values);
}

void URelaySettingsViewModel::SetMasterVolume(float Volume)
{
	if (bSettingValues)
	{
		return;
	}

	FValues Values = GetEdited();
	Values.MasterVolume = FMath::Clamp(Volume, 0.f, 1.f);
	SetEdited(Values);
}

void URelaySettingsViewModel::SetLanguageIndex(int32 Index)
{
	if (bSettingValues)
	{
		return;
	}

	FValues Values = GetEdited();
	Values.LanguageIndex = FMath::Clamp(Index, 0, FMath::Max(0, GetDefault<URelayUISettings>()->SupportedCultures.Num() - 1));
	SetEdited(Values);
}

URelaySettingsViewModel::FValues URelaySettingsViewModel::GetEdited() const
{
	FValues Values;
	Values.QualityLevel = QualityLevel;
	Values.ResolutionScale = ResolutionScale;
	Values.bVSync = bVSync;
	Values.MasterVolume = MasterVolume;
	Values.LanguageIndex = LanguageIndex;
	return Values;
}

void URelaySettingsViewModel::SetEdited(const FValues& NewValues)
{
	// Привязка ставит значение в ползунок, а USlider::SetValue сразу сообщает об изменении обратно.
	// Без защиты вложенный вызов собрал бы значения из полей, которые ещё не обновлены
	TGuardValue<bool> SettingValuesGuard(bSettingValues, true);

	UE_MVVM_SET_PROPERTY_VALUE(QualityLevel, NewValues.QualityLevel);
	UE_MVVM_SET_PROPERTY_VALUE(ResolutionScale, NewValues.ResolutionScale);
	UE_MVVM_SET_PROPERTY_VALUE(bVSync, NewValues.bVSync);
	UE_MVVM_SET_PROPERTY_VALUE(MasterVolume, NewValues.MasterVolume);
	UE_MVVM_SET_PROPERTY_VALUE(LanguageIndex, NewValues.LanguageIndex);

	FNumberFormattingOptions Percent;
	Percent.SetMaximumFractionalDigits(0);

	// Ползунок хранит долю допустимого диапазона, а показываем настоящий процент разрешения
	float ScalePercent = NewValues.ResolutionScale * 100.f;
	if (const UGameUserSettings* Settings = UGameUserSettings::GetGameUserSettings())
	{
		float CurrentNormalized = 0.f, CurrentValue = 0.f, MinValue = 0.f, MaxValue = 0.f;
		Settings->GetResolutionScaleInformationEx(CurrentNormalized, CurrentValue, MinValue, MaxValue);
		ScalePercent = FMath::Lerp(MinValue, MaxValue, NewValues.ResolutionScale);
	}

	UE_MVVM_SET_PROPERTY_VALUE(ResolutionScaleText, FText::AsPercent(ScalePercent / 100.f, &Percent));
	UE_MVVM_SET_PROPERTY_VALUE(MasterVolumeText, FText::AsPercent(NewValues.MasterVolume, &Percent));
	UE_MVVM_SET_PROPERTY_VALUE(bHasChanges, !(NewValues == Saved));
}

#undef LOCTEXT_NAMESPACE
