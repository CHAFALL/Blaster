

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Pickup.generated.h"

UCLASS()
class BLASTER_API APickup : public AActor
{
	GENERATED_BODY()
	
public:	
	APickup();
	virtual void Tick(float DeltaTime) override;
	virtual void Destroyed() override;

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

	UPROPERTY(EditAnywhere)
	float BaseTurnRate = 45.f;

private:
	UPROPERTY(EditAnywhere)
	class USphereComponent* OverlapSphere;

	UPROPERTY(EditAnywhere)
	class USoundCue* PickupSound;

	UPROPERTY(EditAnywhere)
	UStaticMeshComponent* PickupMesh;

	UPROPERTY(VisibleAnywhere)
	class UNiagaraComponent* PickupEffectComponent;

	// 먹을 때 뜰 이펙트
	UPROPERTY(EditAnywhere)
	class UNiagaraSystem* PickupEffect;

	FTimerHandle BindOverlapTimer;
	float BindOverlapTime = 0.25f;
	// 이렇게 하면 캐릭터와 겹쳤을 때 픽업트럭이 즉시 파괴되지 않음. (스폰될 때 충돌이 발생하지 않으므로)
	void BindOverlapTimerFinished();

public:	

};
