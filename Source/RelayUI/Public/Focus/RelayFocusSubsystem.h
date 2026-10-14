#pragma once

#include "Subsystems/LocalPlayerSubsystem.h"
#include "Containers/Ticker.h"
#include "Templates/SharedPointer.h"
#include "RelayFocusSubsystem.generated.h"

class FRelayPointerMotionTracker;
class FWeakWidgetPath;
class FWidgetPath;
class SWidget;
class UWidget;
struct FFocusEvent;

/**
 * Keeps keyboard and gamepad focus where the player put it when the input device changes.
 *
 * On every switch between gamepad and keyboard or mouse, CommonUI moves focus back to a widget it remembered
 * for the active screen. It remembers that widget before Slate moves focus, so without this subsystem it
 * restores the widget focus has just left. Also tells widgets whether the mouse really moved this frame,
 * and returns focus to the active screen if it lands on a screen that does not take input (a menu under a dialog).
 */
UCLASS()
class RELAYUI_API URelayFocusSubsystem : public ULocalPlayerSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	/** Makes CommonUI remember the widget that has focus now. Call it after focus has moved. */
	void RefreshRememberedFocus() const;

	/** True if this player's mouse really moved this frame (see FRelayPointerMotionTracker). */
	bool HasPointerMovedThisFrame() const;

	/** True if the widget belongs to the screen that takes input now (the leafmost active CommonUI screen). */
	bool IsInActiveScreen(const UWidget& Widget) const;

private:
	void KeepFocusInActiveScreen() const;
	void HandleSlateFocusChanging(const FFocusEvent& FocusEvent, const FWeakWidgetPath& OldFocusedWidgetPath, const TSharedPtr<SWidget>& OldFocusedWidget, const FWidgetPath& NewFocusedWidgetPath, const TSharedPtr<SWidget>& NewFocusedWidget);

	FDelegateHandle SlateFocusChangingHandle;
	FTSTicker::FDelegateHandle RefreshTicker;
	TSharedPtr<FRelayPointerMotionTracker> PointerMotionTracker;
};
