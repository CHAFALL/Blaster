


#include "PlayerState/BlasterPlayerState.h"
#include "Blaster/Character/MyBlasterCharacter.h"
#include "Blaster/PlayerController/BlasterPlayerController.h"
#include "Net/UnrealNetwork.h"

void ABlasterPlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ABlasterPlayerState, Defeats);
	DOREPLIFETIME(ABlasterPlayerState, Team);

}

void ABlasterPlayerState::AddToScore(float ScoreAmount)
{
	SetScore(GetScore() + ScoreAmount);
	Character = Character == nullptr ? Cast<AMyBlasterCharacter>(GetPawn()) : Character;
	if (Character)
	{
		Controller = Controller == nullptr ? Cast<ABlasterPlayerController>(Character->Controller) : Controller;
	}
	if (Controller)
	{
		Controller->SetHUDScore(GetScore());
	}
}



void ABlasterPlayerState::OnRep_Score()
{
	Super::OnRep_Score();

	Character = Character == nullptr ? Cast<AMyBlasterCharacter>(GetPawn()) : Character;
	if (Character)
	{
		Controller = Controller == nullptr ? Cast<ABlasterPlayerController>(Character->Controller) : Controller;
	}
	// 컨트롤러를 통해 HUD 접근 가능
	if (Controller)
	{
		Controller->SetHUDScore(GetScore()); // 복제된 값 사용.
	}
}

void ABlasterPlayerState::AddToDefeats(int32 DefeatsAmount)
{
	Defeats += DefeatsAmount;
	Character = Character == nullptr ? Cast<AMyBlasterCharacter>(GetPawn()) : Character;
	if (Character)
	{
		Controller = Controller == nullptr ? Cast<ABlasterPlayerController>(Character->Controller) : Controller;
	}
	if (Controller)
	{
		Controller->SetHUDDefeats(Defeats);
	}
}

void ABlasterPlayerState::OnRep_Defeats()
{
	Character = Character == nullptr ? Cast<AMyBlasterCharacter>(GetPawn()) : Character;
	if (Character)
	{
		Controller = Controller == nullptr ? Cast<ABlasterPlayerController>(Character->Controller) : Controller;
	}
	if (Controller)
	{
		Controller->SetHUDDefeats(Defeats);
	}
}

void ABlasterPlayerState::SetTeam(ETeam TeamToSet)
{
	Team = TeamToSet;

	AMyBlasterCharacter* BCharacter = Cast<AMyBlasterCharacter>(GetPawn());
	if (BCharacter)
	{
		BCharacter->SetTeamColor(Team);
	}
}


void ABlasterPlayerState::OnRep_Team()
{
	AMyBlasterCharacter* BCharacter = Cast<AMyBlasterCharacter>(GetPawn());
	if (BCharacter)
	{
		BCharacter->SetTeamColor(Team);
	}
}

