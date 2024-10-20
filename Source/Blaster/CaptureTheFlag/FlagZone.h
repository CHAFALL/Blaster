

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Blaster/BlasterTypes/Team.h"
#include "FlagZone.generated.h"

UCLASS()
class BLASTER_API AFlagZone : public AActor
{
	GENERATED_BODY()
	
public:	
	AFlagZone();

	UPROPERTY(EditAnywhere)
	ETeam Team;

protected:
	virtual void BeginPlay() override;

	UFUNCTION()
	virtual void OnSphereOverlap(
		UPrimitiveComponent* OverlappedComponent, // 충돌을 감지한 컴포넌트
		AActor* OtherActor, // 현재 객체의 충돌체와 겹친 액터
		UPrimitiveComponent* OtherComp, // 충돌한 객체의 컴포넌트
		int32 OtherBodyIndex, // 충돌한 바디의 인덱스 
		bool bFromSweep,// 충돌이 Sweep으로 발생했는지 여부를 나타냄, true면 객체가 물리적으로 이동하면서 충돌 감지, false면 정적인 상태에서 충돌 발생했음
		const FHitResult& SweepResult // 충돌에 대한 세부 정보가 담긴 구조체
	);

private:
	UPROPERTY(EditAnywhere)
	class USphereComponent* ZoneSphere;

public:	

};
