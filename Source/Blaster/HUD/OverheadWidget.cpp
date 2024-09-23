// Fill out your copyright notice in the Description page of Project Settings.


#include "HUD/OverheadWidget.h"
#include "Components/TextBlock.h"
#include "GameFramework/PlayerState.h"

void UOverheadWidget::SetDisplayText(FString TextToDisplay)
{
	if (DisplayText)
	{
		DisplayText->SetText(FText::FromString(TextToDisplay));
	}
}

void UOverheadWidget::ShowPlayerNetRole(APawn* InPawn)
{
	ENetRole LocalRole = InPawn->GetLocalRole();
	FString LocalRoleStr;
	switch (LocalRole)
	{
	case ROLE_None:
		LocalRoleStr = FString("None");
		break;
	case ROLE_SimulatedProxy:
		LocalRoleStr = FString("SimulatedProxy");
		break;
	case ROLE_AutonomousProxy:
		LocalRoleStr = FString("AutonomousProxy");
		break;
	case ROLE_Authority:
		LocalRoleStr = FString("Authority");
		break;
	}

	ENetRole RemoteRole = InPawn->GetRemoteRole();
	FString RemoteRoleStr;
	switch (RemoteRole)
	{
	case ROLE_None:
		RemoteRoleStr = FString("None");
		break;
	case ROLE_SimulatedProxy:
		RemoteRoleStr = FString("SimulatedProxy");
		break;
	case ROLE_AutonomousProxy:
		RemoteRoleStr = FString("AutonomousProxy");
		break;
	case ROLE_Authority:
		RemoteRoleStr = FString("Authority");
		break;
	}

	// 참고
	// 플레이어 이름을 가져옴 (PlayerState에서)
	/*FString PlayerName = "Unknown";
	if (APlayerState* PlayerState = InPawn->GetPlayerState<APlayerState>())
	{
		PlayerName = PlayerState->GetPlayerName();
	}*/

	FString RoleStr = FString::Printf(TEXT("Local Role: %s\n Remote Role: %s"), *LocalRoleStr, *RemoteRoleStr);
	SetDisplayText(RoleStr);
}

void UOverheadWidget::NativeDestruct()
{
	RemoveFromParent();
	Super::NativeDestruct();
}