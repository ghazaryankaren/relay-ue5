#include "Router/RelayScreenRegistry.h"

#include "RelayUITags.h"

#if WITH_EDITOR
#include "Misc/DataValidation.h"
#include "Misc/PackageName.h"
#endif

#define LOCTEXT_NAMESPACE "RelayScreenRegistry"

const FRelayScreenEntry* URelayScreenRegistry::FindScreen(FGameplayTag ScreenTag) const
{
	return Screens.Find(ScreenTag);
}

#if WITH_EDITOR
EDataValidationResult URelayScreenRegistry::IsDataValid(FDataValidationContext& Context) const
{
	EDataValidationResult Result = Super::IsDataValid(Context);

	for (const TPair<FGameplayTag, FRelayScreenEntry>& Pair : Screens)
	{
		const FText ScreenName = FText::FromName(Pair.Key.GetTagName());
		const FRelayScreenEntry& Entry = Pair.Value;

		if (!Pair.Key.IsValid())
		{
			Context.AddError(LOCTEXT("EmptyScreenTag", "Screen entry has no tag."));
			Result = EDataValidationResult::Invalid;
		}

		if (Entry.WidgetClass.IsNull())
		{
			Context.AddError(FText::Format(LOCTEXT("NoWidgetClass", "Screen {0} has no widget class."), ScreenName));
			Result = EDataValidationResult::Invalid;
		}
		else if (!FPackageName::DoesPackageExist(Entry.WidgetClass.ToSoftObjectPath().GetLongPackageName()))
		{
			// Ссылка на удалённый или переименованный без редиректора виджет
			Context.AddError(FText::Format(LOCTEXT("MissingWidgetClass", "Screen {0} points to a missing widget {1}."),
				ScreenName, FText::FromString(Entry.WidgetClass.ToString())));
			Result = EDataValidationResult::Invalid;
		}

		// Фильтр Categories в пикере пропускает и сам родительский тег UI.Layer, поэтому сверяем со списком слоёв
		if (!RelayUITags::AllLayers().Contains(Entry.Layer))
		{
			Context.AddError(FText::Format(LOCTEXT("UnknownLayer", "Screen {0} has no layer or an unknown layer {1}."),
				ScreenName, FText::FromName(Entry.Layer.GetTagName())));
			Result = EDataValidationResult::Invalid;
		}
	}

	if (Result == EDataValidationResult::NotValidated)
	{
		Result = EDataValidationResult::Valid;
	}

	return Result;
}
#endif

#undef LOCTEXT_NAMESPACE
