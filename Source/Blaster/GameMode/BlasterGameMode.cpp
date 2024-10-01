


#include "GameMode/BlasterGameMode.h"
#include "Blaster/Character/MyBlasterCharacter.h"
#include "Blaster/PlayerController/BlasterPlayerController.h"

void ABlasterGameMode::PlayerEliminated(AMyBlasterCharacter* ElimmedCharacter, ABlasterPlayerController* VictimController, ABlasterPlayerController* AttackerController)
{
	// 서버에서만 실행됨.
	if (ElimmedCharacter)
	{
		ElimmedCharacter->Elim();
	}
}
