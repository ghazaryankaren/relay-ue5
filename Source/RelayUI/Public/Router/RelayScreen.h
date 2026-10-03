#pragma once

#include "CommonActivatableWidget.h"
#include "GameplayTagContainer.h"
#include "RelayScreen.generated.h"

class UInputAction;
class URelayUIRouterSubsystem;

/** Who receives input while the screen is on top. */
UENUM(BlueprintType)
enum class ERelayScreenInputMode : uint8
{
	/** Only UI receives input. The cursor is free, game input is blocked. */
	Menu,
	/** The game receives input. For HUD and anything that must not take control away. */
	Game
};

/** An input action that opens another screen while this screen is on top. */
USTRUCT(BlueprintType)
struct RELAYUI_API FRelayScreenShortcut
{
	GENERATED_BODY()

	/** Input action. Its keys must be in an applied mapping context (IMC_UI). */
	UPROPERTY(EditDefaultsOnly, Category = "Shortcut")
	TObjectPtr<const UInputAction> Action;

	/** Screen to open. */
	UPROPERTY(EditDefaultsOnly, Category = "Shortcut", meta = (Categories = "UI.Screen"))
	FGameplayTag Screen;
};

/**
 * Base class of every Relay screen. Screens never reference each other: they ask the router for a screen by tag.
 * A CommonUI stack reuses the widget object of a closed screen, so reset per-show state in NativeOnActivated.
 */
UCLASS(Abstract, Blueprintable, meta = (DisableNativeTick))
class RELAYUI_API URelayScreen : public UCommonActivatableWidget
{
	GENERATED_BODY()

public:
	URelayScreen(const FObjectInitializer& ObjectInitializer);

	/** Builds the input config from InputMode. A Blueprint override of the input config is ignored. */
	virtual TOptional<FUIInputConfig> GetDesiredInputConfig() const override;

	/**
	 * Called by the router exactly once per close request (Back, a close button, Replace).
	 * Return true to close now. Return false to stay open and handle the request yourself,
	 * for example to ask about unsaved changes and close later through the router.
	 */
	virtual bool HandleCloseRequest() { return true; }

protected:
	virtual void NativeOnInitialized() override;
	virtual bool NativeOnHandleBackAction() override;

	URelayUIRouterSubsystem* GetRouter() const;

	/** Input mode applied while this screen is the top active screen. HUD screens set Game. */
	UPROPERTY(EditDefaultsOnly, Category = "Input")
	ERelayScreenInputMode InputMode = ERelayScreenInputMode::Menu;

	/** Input actions that open other screens while this screen is on top. Example: the HUD opens the menu on OpenMenu. */
	UPROPERTY(EditDefaultsOnly, Category = "Input")
	TArray<FRelayScreenShortcut> Shortcuts;

private:
	void HandleShortcut(FGameplayTag ScreenTag);
};
