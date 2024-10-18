

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "LagCompensationComponent.generated.h"


USTRUCT(BlueprintType)
struct FBoxInformation
{
	GENERATED_BODY()

	UPROPERTY()
	FVector Location;

	UPROPERTY()
	FRotator Rotation;

	UPROPERTY()
	FVector BoxExtent;

};

USTRUCT(BlueprintType)
struct FFramePackage
{
	GENERATED_BODY()

	UPROPERTY()
	float Time;

	// 포인터 저장 x, 그냥 주소일 뿐.
	UPROPERTY()
	TMap<FName, FBoxInformation> HitBoxInfo;

	UPROPERTY()
	AMyBlasterCharacter* Character;

};

USTRUCT(BlueprintType)
struct FServerSideRewindResult
{
	GENERATED_BODY()

	UPROPERTY()
	bool bHitConfirmed;

	UPROPERTY()
	bool bHeadShot;
};

USTRUCT(BlueprintType)
struct FShotgunServerSideRewindResult
{
	GENERATED_BODY()

	UPROPERTY()
	TMap<AMyBlasterCharacter*, uint32> HeadShots;

	UPROPERTY()
	TMap<AMyBlasterCharacter*, uint32> BodyShots;
	
};


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class BLASTER_API ULagCompensationComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	ULagCompensationComponent();
	friend class AMyBlasterCharacter;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	void ShowFramePackage(const FFramePackage& Package, const FColor& Color);
	
	/**
	* Hitscan
	*/

	FServerSideRewindResult ServerSideRewind(class AMyBlasterCharacter* HitCharacter, const FVector_NetQuantize& TraceStart, const FVector_NetQuantize& HitLocation, float HitTime);
	
	/**
	* Projectile
	*/

	FServerSideRewindResult ProjectileServerSideRewind(AMyBlasterCharacter* HitCharacter, const FVector_NetQuantize& TraceStart, const FVector_NetQuantize100& InitialVelocity, float HitTime);


	/**
	* Shotgun
	*/
	
	FShotgunServerSideRewindResult ShotgunServerSideRewind(const TArray<AMyBlasterCharacter*>& HitCharacters, const FVector_NetQuantize& TraceStart, const TArray<FVector_NetQuantize>& HitLocations, float HitTime);
	
	UFUNCTION(Server, Reliable)
	void ServerScoreRequest(AMyBlasterCharacter* HitCharacter, const FVector_NetQuantize& TraceStart, const FVector_NetQuantize& HitLocation, float HitTime);
	
	UFUNCTION(Server, Reliable)
	void ProjectileServerScoreRequest(AMyBlasterCharacter* HitCharacter, const FVector_NetQuantize& TraceStart, const FVector_NetQuantize100& InitialVelocity, float HitTime);
	
	UFUNCTION(Server, Reliable)
	void ShotgunServerScoreRequest(const TArray<AMyBlasterCharacter*>& HitCharacters, const FVector_NetQuantize& TraceStart, const TArray<FVector_NetQuantize>& HitLocations, float HitTime);

protected:
	virtual void BeginPlay() override;
	void SaveFramePackage(FFramePackage& Package);
	FFramePackage InterpBetweenFrames(const FFramePackage& OlderFrame, const FFramePackage& YoungerFrame, float HitTime);
	
	void CacheBoxPositions(AMyBlasterCharacter* HitCharacter, FFramePackage& OutFramePackage);
	void MoveBoxes(AMyBlasterCharacter* HitCharacter, const FFramePackage& Package);
	void ResetHitBoxes(AMyBlasterCharacter* HitCharacter, const FFramePackage& Package);
	void EnableCharacterMeshCollision(AMyBlasterCharacter* HitCharacter, ECollisionEnabled::Type CollisionEnabled);
	void SaveFramePackage();
	FFramePackage GetFrameToCheck(AMyBlasterCharacter* HitCharacter, float HitTime);


	/**
	* Hitscan
	*/

	// 참고 샷건할 때 Package안에 HitCharacter가 필요해서 구조를 조금 바꿨음 (그래서 여기서도 AMyBlasterCharacter* HitCharacter가 필요없지만 남겨두겠다.)
	FServerSideRewindResult ConfirmHit(const FFramePackage& Package, AMyBlasterCharacter* HitCharacter, const FVector_NetQuantize& TraceStart, const FVector_NetQuantize& HitLocation);


	/**
	* Projectile
	*/

	FServerSideRewindResult ProjectileConfirmHit(const FFramePackage& Package, AMyBlasterCharacter* HitCharacter, const FVector_NetQuantize& TraceStart, const FVector_NetQuantize100& InitialVelocity, float HitTime);

	/**
	* Shotgun
	*/

	FShotgunServerSideRewindResult ShotgunConfirmHit(const TArray<FFramePackage>& FramePackages, const FVector_NetQuantize& TraceStart, const TArray<FVector_NetQuantize>& HitLocations);

private:
	UPROPERTY()
	AMyBlasterCharacter* Character;

	UPROPERTY()
	class ABlasterPlayerController* Controller;

	// 프레임 패키지는 일반적으로 시간 순서대로 저장하고,
	// 이전 프레임들을 삭제하며 새로운 프레임을 추가하는 작업이 "빈번하게" 발생
	// 자료구조 TArray는 별로, TDoubleLinkedList를 이용하자 (Head, Tail 접근이 용이)

	// Head가 가장 최신, Tail이 가장 오래된.
	TDoubleLinkedList<FFramePackage> FrameHistory;

	UPROPERTY(EditAnywhere)
	float MaxRecordTime = 4.f; // 이거 너무 낮추니 안되는 문제 발생. (처음에 0.4f로 했었음.)


public:	

		
};

// Frame History
// 최근 캐릭터 위치들을 기록해야됨
// 어떻게 저장할 것인가?? 벡터로 위치를 저장하는 것만으로 충분할까? No (앉아있으면 우얌)
// 그럼 얼마나 저장해야 되는데?? Box를 이용하자, 캐릭터에 Box를 추가하고 위치와 회전 위치, 상자 넓이를 저장 가능
// 얼마나 자세히 하냐에 따라 비용이 많이 듬.
// (캡슐은 좀 그럼, 팔이 메쉬 밖으로 튀어나온다거나 그런 경우가 many해서 근사치가 별로임.)



// 클라이언트 측 예측. (일단 움직이고 용서)
// 지터가 생긴 이유 - 서버에서 데이터를 받을 때마다 과거로부터 데이터를 받는다는 걸 기억해야 하기 때문.
// 캐릭터 이동 구성 요소는 이미 적용이 되어있다. 이 기술을 다른 분야에도 적용하겠다.

// ------
// 서버 측 되감기로 알려진 지연 보상 기술.
// 서버 측 되감기 기술은 네트워크 지연으로 인해 발생하는 공격 판정의 불공평성을 보상하기 위한 방법으로, 
// 서버는 시간을 되돌려 특정 시점에서 캐릭터들의 위치를 계산하여 공격의 적중 여부를 판단
// 서버는 업데이트를 받는데 걸리는 시간을 아는 한 계산을 수행 가능.

// 장점:  네트워크 지연을 보상해 핑이 높은 플레이어도 자신의 공격이 정확하게 판정되도록 합니다.
// 단점 :핑이 낮은 플레이어는 되감기 기술이 불필요하다고 느낄 수 있고,
// 네트워크 지연으로 인해 실제 상황과 판정이 일치하지 않는 불공정한 상황이 발생할 수 있다.
// 특히, 엄폐물 뒤에 숨어도 피격 판정이 날 수 있다. (난 분명 숨었다고 생각했는데 판정은 좀 늦게 나서 억울해 하는 상황 발생.)
// (우리 기기와 서버와의 딜레이가 있는 것과 마찬가지로 상대 기기와도 서버가 딜레이가 있는데
// 판단하는 것은 서버가 하는 것이어서 발생하는 문제)
// 따라서 특정 지연 시간까지 슈팅 게임에서 서버 측 되감기를 사용하는 것이 좋다.

// (핑에 따른 입장)
// 핑이 낮은 플레이어는 자신이 실시간으로 게임을 플레이하고 있으며,
// 총알이 즉각적으로 적중해야 한다고 느끼기 때문에
// 되감기 기술로 인해 발생하는 딜레이나 불공정성에 불만을 가질 수 있다.
// 핑이 높은 플레이어는 네트워크 지연으로 인해 자신의 공격이 적중하지 않는 경우를 방지하기 위해
// 되감기 기술에 의존하게 된다.
// 그렇다고 핑이 낮은 플레이어만 게임에 참가하게 한다면 많은 플레이어를 잃게 된다.

// SSR은 뼈 이름이 뭔지와 같은 것을 파악할 필요가 없음. (헤드샷 part.)