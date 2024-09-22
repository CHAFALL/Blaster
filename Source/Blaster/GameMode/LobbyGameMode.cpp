// Fill out your copyright notice in the Description page of Project Settings.


#include "LobbyGameMode.h"
#include "GameFramework/GameStateBase.h"

// 특정 수 이상의 플레이어가 모이면 새로운 곳으로 여행
void ALobbyGameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);
	// 게임에 얼마나 많은 플레이어가 있는지
	int32 NumberOfPlayers = GameState.Get()->PlayerArray.Num();

	if (NumberOfPlayers == 2)
	{
		UWorld* World = GetWorld();
		if (World)
		{
			bUseSeamlessTravel = true; // 원할하게 여행하기
			World->ServerTravel(FString("/Game/Maps/BlasterMap?listen"));
		}
	}
}
