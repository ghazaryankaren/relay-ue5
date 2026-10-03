#include "CoreMinimal.h"

#if !UE_BUILD_SHIPPING

#include "Engine/LocalPlayer.h"
#include "Engine/World.h"
#include "GameplayTagContainer.h"
#include "HAL/IConsoleManager.h"
#include "RelayUILog.h"
#include "Router/RelayUIRouterSubsystem.h"

// Отладочные команды роутера: открыть или закрыть экран без кнопок, в PIE и в запуске игры без рендера
namespace RelayUIConsoleCommands
{
	using FRouterCommand = TFunctionRef<void(URelayUIRouterSubsystem&, FGameplayTag)>;

	static void Run(const TArray<FString>& Args, UWorld* World, const TCHAR* Usage, FRouterCommand Command)
	{
		ULocalPlayer* LocalPlayer = World ? World->GetFirstLocalPlayerFromController() : nullptr;
		URelayUIRouterSubsystem* Router = ULocalPlayer::GetSubsystem<URelayUIRouterSubsystem>(LocalPlayer);
		const FGameplayTag Tag = Args.Num() > 0 ? FGameplayTag::RequestGameplayTag(FName(*Args[0]), false) : FGameplayTag();

		if (!Router || !Tag.IsValid())
		{
			UE_LOG(LogRelayUI, Warning, TEXT("Usage: %s. Router or tag not found."), Usage);
			return;
		}

		Command(*Router, Tag);
	}

	static FAutoConsoleCommandWithWorldAndArgs PushCommand(
		TEXT("Relay.UI.Push"),
		TEXT("Opens a screen on top of its layer. Example: Relay.UI.Push UI.Screen.Shop"),
		FConsoleCommandWithWorldAndArgsDelegate::CreateLambda([](const TArray<FString>& Args, UWorld* World)
		{
			Run(Args, World, TEXT("Relay.UI.Push <UI.Screen tag>"), [](URelayUIRouterSubsystem& Router, FGameplayTag Tag)
			{
				Router.PushScreen(Tag);
			});
		}));

	static FAutoConsoleCommandWithWorldAndArgs ReplaceCommand(
		TEXT("Relay.UI.Replace"),
		TEXT("Opens a screen in place of the top screen of its layer. Example: Relay.UI.Replace UI.Screen.Settings"),
		FConsoleCommandWithWorldAndArgsDelegate::CreateLambda([](const TArray<FString>& Args, UWorld* World)
		{
			Run(Args, World, TEXT("Relay.UI.Replace <UI.Screen tag>"), [](URelayUIRouterSubsystem& Router, FGameplayTag Tag)
			{
				Router.ReplaceScreen(Tag);
			});
		}));

	static FAutoConsoleCommandWithWorldAndArgs CloseCommand(
		TEXT("Relay.UI.Close"),
		TEXT("Closes the top screen of a layer through the router (asks the screen, ignores its Back handler flag). Example: Relay.UI.Close UI.Layer.Menu"),
		FConsoleCommandWithWorldAndArgsDelegate::CreateLambda([](const TArray<FString>& Args, UWorld* World)
		{
			Run(Args, World, TEXT("Relay.UI.Close <UI.Layer tag>"), [](URelayUIRouterSubsystem& Router, FGameplayTag Tag)
			{
				Router.RequestCloseTopScreen(Tag);
			});
		}));
}

#endif
