

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "BlasterPlayerState.generated.h"

/**
 * 
 */
UCLASS()
class BLASTER_API ABlasterPlayerState : public APlayerState
{
	GENERATED_BODY()

public:
	virtual void OnRep_Score() override; 
	void AddToScore(float ScoreAmount);

private:
	class AMyBlasterCharacter* Character;
	class ABlasterPlayerController* Controller;
	
};

// 플레이어 컨트롤러는 playerstate에 접속하는 기능을 기본 제공.
// 하지만 playerstate는 플레이어 컨트롤러에 접속하는 직접적인 기능이 없음.
// 플레이어 상태와 관련된 pawn에는 엑세스 가능하다

// PlayerController가 주로 클라이언트 측 논리를 처리하는 반면, PlayerState는 서버와 클라이언트 모두에서 플레이어의 상태 정보를 추적하는 데 사용되기 때문에 의도된 설계