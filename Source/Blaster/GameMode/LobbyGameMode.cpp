// Fill out your copyright notice in the Description page of Project Settings.


#include "LobbyGameMode.h"
#include "GameFramework/GameStateBase.h"

// Ư�� �� �̻��� �÷��̾ ���̸� ���ο� ������ ����
void ALobbyGameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);
	// ���ӿ� �󸶳� ���� �÷��̾ �ִ���
	int32 NumberOfPlayers = GameState.Get()->PlayerArray.Num();

	if (NumberOfPlayers == 2)
	{
		UWorld* World = GetWorld();
		if (World)
		{
			bUseSeamlessTravel = true; // �����ϰ� �����ϱ�
			World->ServerTravel(FString("/Game/Maps/BlasterMap?listen"));
		}
	}
}
