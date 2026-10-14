#pragma once

#include "Subsystems/LocalPlayerSubsystem.h"
#include "GameplayTagContainer.h"
#include "Templates/SharedPointer.h"
#include "UObject/StrongObjectPtr.h"
#include "ViewModel/RelayViewModelBase.h"
#include "RelayUIRouterSubsystem.generated.h"

class URelayRootLayout;
class URelayScreen;
class URelayScreenRegistry;
struct FRelayScreenEntry;
struct FStreamableHandle;

/**
 * Screen router of one local player. The only place that opens and closes screens.
 *
 * The UI is built in PlayerControllerChanged, which runs before the world's BeginPlay and before the pawn exists
 * (on a client, inside packet processing). Screens must not rely on the pawn or the GameState when they open.
 */
UCLASS()
class RELAYUI_API URelayUIRouterSubsystem : public ULocalPlayerSubsystem
{
	GENERATED_BODY()

public:
	/**
	 * Opens a screen on top of its layer. The screen class is loaded asynchronously if it is not in memory yet.
	 * Context reaches URelayScreen::SetScreenContext before the screen activates; the router keeps it alive while the class loads.
	 */
	void PushScreen(FGameplayTag ScreenTag, URelayViewModelBase* Context = nullptr);

	/** Opens a screen in place of the top screen of its layer. Back then returns to the screen below the replaced one. */
	void ReplaceScreen(FGameplayTag ScreenTag, URelayViewModelBase* Context = nullptr);

	/** Closes the screen if it agrees (URelayScreen::HandleCloseRequest). Also cancels screens still loading on its layer and on the layers above it. */
	void RequestCloseScreen(URelayScreen& Screen);

	/** Debug and tools: closes the top screen of a layer through RequestCloseScreen, or cancels the screen still loading there. */
	void RequestCloseTopScreen(FGameplayTag LayerTag);

	/** True if the screen is in one of the layer stacks: on top, or covered by another screen of its layer. */
	bool IsScreenOpen(const URelayScreen& Screen) const;

	virtual void PlayerControllerChanged(APlayerController* NewPlayerController) override;
	virtual void Deinitialize() override;

private:
	enum class EScreenOp : uint8
	{
		Push,
		Replace
	};

	struct FPendingScreen
	{
		FGameplayTag ScreenTag;
		EScreenOp Op = EScreenOp::Push;
		uint32 RequestId = 0;
		double RequestTime = 0.0;
		TSharedPtr<FStreamableHandle> Handle;

		// Keeps the context alive while the screen class loads: the requester may have released it
		TStrongObjectPtr<URelayViewModelBase> Context;
	};

	void OpenScreen(FGameplayTag ScreenTag, EScreenOp Op, URelayViewModelBase* Context);
	void OnScreenClassLoaded(FGameplayTag LayerTag, uint32 RequestId);
	void ShowScreen(FGameplayTag ScreenTag, const FRelayScreenEntry& Entry, UClass* ScreenClass, EScreenOp Op, URelayViewModelBase* Context, double RequestTime);
	void CancelPendingScreen(FGameplayTag LayerTag);
	void CancelAllPendingScreens();
	void CloseAllScreensOnLayer(FGameplayTag LayerTag);

	bool CreateRootLayout(APlayerController* PlayerController);
	void DestroyRootLayout();
	void SetUpPlayerInput();
	bool IsUIOnScreen() const;
	bool HasOtherActiveScreen(const URelayScreen& Screen) const;

	UPROPERTY(Transient)
	TObjectPtr<URelayRootLayout> RootLayout;

	// The controller RootLayout was built for. GetOwningPlayer on the layout cannot answer this:
	// it returns the player's current controller, which is already the new one
	TWeakObjectPtr<APlayerController> LayoutController;

	UPROPERTY(Transient)
	TObjectPtr<const URelayScreenRegistry> ScreenRegistry;

	// At most one screen waiting for its class to load per layer
	TMap<FGameplayTag, FPendingScreen> PendingScreens;
	uint32 LastRequestId = 0;
};
