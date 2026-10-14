#include "CoreMinimal.h"

#if !UE_BUILD_SHIPPING

#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "HAL/IConsoleManager.h"
#include "RelayLog.h"
#include "Shop/RelayShopCatalog.h"
#include "Shop/RelayWalletComponent.h"

// Отладочные команды кошелька: проверить покупку без UI и выдать монеты, чтобы проверить магазин
namespace RelayShopConsoleCommands
{
	static URelayWalletComponent* FindWallet(UWorld* World)
	{
		URelayWalletComponent* Wallet = World ? URelayWalletComponent::FindForPlayer(World->GetFirstPlayerController()) : nullptr;
		if (!Wallet)
		{
			UE_LOG(LogRelay, Warning, TEXT("No wallet: the first player has no PlayerState with a wallet."));
		}
		return Wallet;
	}

	static FAutoConsoleCommandWithWorldAndArgs GiveCommand(
		TEXT("Relay.Shop.Give"),
		TEXT("Adds coins to the first player. Example: Relay.Shop.Give 500"),
		FConsoleCommandWithWorldAndArgsDelegate::CreateLambda([](const TArray<FString>& Args, UWorld* World)
		{
			if (URelayWalletComponent* Wallet = FindWallet(World))
			{
				Wallet->AddMoney(Args.Num() > 0 ? FCString::Atoi(*Args[0]) : 1000);
				UE_LOG(LogRelay, Log, TEXT("Balance %d."), Wallet->GetBalance());
			}
		}));

	static FAutoConsoleCommandWithWorldAndArgs BuyCommand(
		TEXT("Relay.Shop.Buy"),
		TEXT("Buys an item for the first player without the UI. Example: Relay.Shop.Buy sword_iron"),
		FConsoleCommandWithWorldAndArgsDelegate::CreateLambda([](const TArray<FString>& Args, UWorld* World)
		{
			URelayWalletComponent* Wallet = FindWallet(World);
			if (!Wallet || Args.Num() == 0)
			{
				return;
			}

			const ERelayPurchaseResult Result = Wallet->TryPurchase(FName(*Args[0]));
			UE_LOG(LogRelay, Log, TEXT("Relay.Shop.Buy %s: %s. Balance %d."), *Args[0], *UEnum::GetValueAsString(Result), Wallet->GetBalance());
		}));

	static FAutoConsoleCommandWithWorldAndArgs ResetCommand(
		TEXT("Relay.Shop.Reset"),
		TEXT("Restores the starting balance of the first player and removes owned items."),
		FConsoleCommandWithWorldAndArgsDelegate::CreateLambda([](const TArray<FString>& Args, UWorld* World)
		{
			if (URelayWalletComponent* Wallet = FindWallet(World))
			{
				Wallet->ResetToStart();
			}
		}));

	static FAutoConsoleCommandWithWorldAndArgs DumpCommand(
		TEXT("Relay.Shop.Dump"),
		TEXT("Logs the balance, owned items and catalog size of the first player."),
		FConsoleCommandWithWorldAndArgsDelegate::CreateLambda([](const TArray<FString>& Args, UWorld* World)
		{
			if (const URelayWalletComponent* Wallet = FindWallet(World))
			{
				const URelayShopCatalog* Catalog = Wallet->GetCatalog();
				UE_LOG(LogRelay, Log, TEXT("Balance %d, owned %d, catalog %d items in %d categories."),
					Wallet->GetBalance(), Wallet->GetNumOwned(),
					Catalog ? Catalog->GetItems().Num() : 0, Catalog ? Catalog->GetCategories().Num() : 0);
			}
		}));
}

#endif
