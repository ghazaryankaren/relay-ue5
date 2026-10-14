#include "CoreMinimal.h"

#if !UE_BUILD_SHIPPING

#include "Engine/LocalPlayer.h"
#include "Engine/World.h"
#include "GameplayTagContainer.h"
#include "HAL/IConsoleManager.h"
#include "RelayUILog.h"
#include "Router/RelayUIRouterSubsystem.h"
#include "Toast/RelayToastSubsystem.h"

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

	static FAutoConsoleCommandWithWorldAndArgs ToastCommand(
		TEXT("Relay.UI.Toast"),
		TEXT("Queues a toast for the first player. Run it several times to see the queue and the limit. Example: Relay.UI.Toast Hello"),
		FConsoleCommandWithWorldAndArgsDelegate::CreateLambda([](const TArray<FString>& Args, UWorld* World)
		{
			ULocalPlayer* LocalPlayer = World ? World->GetFirstLocalPlayerFromController() : nullptr;
			if (URelayToastSubsystem* Toasts = ULocalPlayer::GetSubsystem<URelayToastSubsystem>(LocalPlayer))
			{
				static int32 ToastNumber = 0;
				const FString Message = Args.Num() > 0 ? FString::Join(Args, TEXT(" ")) : FString::Printf(TEXT("Toast %d"), ++ToastNumber);
				Toasts->ShowToast(FText::AsCultureInvariant(Message));
			}
		}));

	// Тосты из консоли по одному не успевают встать в очередь: пока набираешь следующую команду, прошлый тост уже исчез
	static FAutoConsoleCommandWithWorldAndArgs ToastBurstCommand(
		TEXT("Relay.UI.ToastBurst"),
		TEXT("Queues several toasts in one frame to show the visible limit and the queue. Example: Relay.UI.ToastBurst 6"),
		FConsoleCommandWithWorldAndArgsDelegate::CreateLambda([](const TArray<FString>& Args, UWorld* World)
		{
			ULocalPlayer* LocalPlayer = World ? World->GetFirstLocalPlayerFromController() : nullptr;
			if (URelayToastSubsystem* Toasts = ULocalPlayer::GetSubsystem<URelayToastSubsystem>(LocalPlayer))
			{
				const int32 Count = Args.Num() > 0 ? FMath::Clamp(FCString::Atoi(*Args[0]), 1, 20) : 6;
				for (int32 Index = 1; Index <= Count; ++Index)
				{
					Toasts->ShowToast(FText::AsCultureInvariant(FString::Printf(TEXT("Toast %d of %d"), Index, Count)));
				}
			}
		}));
}

#endif
