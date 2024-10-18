

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "Blaster/BlasterTypes/Team.h"
#include "BlasterPlayerState.generated.h"

/**
 * 
 */
UCLASS()
class BLASTER_API ABlasterPlayerState : public APlayerState
{
	GENERATED_BODY()

public:
	virtual void GetLifetimeReplicatedProps(TArray< FLifetimeProperty >& OutLifetimeProps) const override;
	
	/**
	* Replication notifies
	*/
	virtual void OnRep_Score() override;

	UFUNCTION()
	virtual void OnRep_Defeats();
	
	void AddToScore(float ScoreAmount);
	void AddToDefeats(int32 DefeatsAmount);

private:
	// 처음에 nullptr로 밀어주기 위해 = nullptr;로 해줘도 되고 언리얼 엔진에서는 새로운 속성을 주면 됨 (UPROPERTY())
	UPROPERTY()
	class AMyBlasterCharacter* Character;
	UPROPERTY()
	class ABlasterPlayerController* Controller;

	// 점수(Score)는 기본 제공.

	UPROPERTY(ReplicatedUsing = OnRep_Defeats)
	int32 Defeats;
	
	UPROPERTY(ReplicatedUsing = OnRep_Team)
	ETeam Team = ETeam::ET_NoTeam;

	UFUNCTION()
	void OnRep_Team();

public:
	FORCEINLINE ETeam GetTeam() const { return Team; }
	void SetTeam(ETeam TeamToSet);


};

// 만약에 위의 정보들을 여기서 관리하지 않고 캐릭터로 관리한다면?
// -> 리스폰 시 데이터 손실 문제가 있음.


// 플레이어 컨트롤러는 playerstate에 접속하는 기능을 기본 제공.
// 하지만 playerstate는 플레이어 컨트롤러에 접속하는 직접적인 기능이 없음.
// 플레이어 상태와 관련된 pawn에는 엑세스 가능하다

// PlayerController가 주로 클라이언트 측 논리를 처리하는 반면, PlayerState는 서버와 클라이언트 모두에서 플레이어의 상태 정보를 추적하는 데 사용되기 때문에 의도된 설계