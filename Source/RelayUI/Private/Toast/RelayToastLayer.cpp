#include "Toast/RelayToastLayer.h"

#include "Components/PanelWidget.h"
#include "Engine/LocalPlayer.h"
#include "RelayUILog.h"
#include "Toast/RelayToast.h"
#include "Toast/RelayToastSubsystem.h"

URelayToastLayer::URelayToastLayer(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
	, ToastPool(*this)
{
}

URelayToast* URelayToastLayer::AddToast(const FText& Message)
{
	if (!ToastClass)
	{
		UE_LOG(LogRelayUI, Error, TEXT("%s: Toast Class is not set."), *GetClass()->GetName());
		return nullptr;
	}

	URelayToast* Toast = ToastPool.GetOrCreateInstance<URelayToast>(ToastClass);
	if (!Toast)
	{
		return nullptr;
	}

	Toast->SetMessage(Message);
	ToastBox->AddChild(Toast);
	return Toast;
}

void URelayToastLayer::RemoveToast(URelayToast& Toast)
{
	ToastBox->RemoveChild(&Toast);
	ToastPool.Release(&Toast);
}

void URelayToastLayer::ReleaseSlateResources(bool bReleaseChildren)
{
	Super::ReleaseSlateResources(bReleaseChildren);
	ToastPool.ReleaseAllSlateResources();
}

void URelayToastLayer::NativeConstruct()
{
	Super::NativeConstruct();

	// Тосты только показываются: слой не должен перехватывать мышь у экранов под ним
	SetVisibility(ESlateVisibility::HitTestInvisible);

	if (URelayToastSubsystem* Toasts = ULocalPlayer::GetSubsystem<URelayToastSubsystem>(GetOwningLocalPlayer()))
	{
		Toasts->RegisterLayer(*this);
	}
}

void URelayToastLayer::NativeDestruct()
{
	if (URelayToastSubsystem* Toasts = ULocalPlayer::GetSubsystem<URelayToastSubsystem>(GetOwningLocalPlayer()))
	{
		Toasts->UnregisterLayer(*this);
	}

	Super::NativeDestruct();
}
