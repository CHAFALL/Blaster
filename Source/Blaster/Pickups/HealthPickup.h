

#pragma once

#include "CoreMinimal.h"
#include "Pickups/Pickup.h"
#include "HealthPickup.generated.h"

/**
 * 
 */
UCLASS()
class BLASTER_API AHealthPickup : public APickup
{
	GENERATED_BODY()
	
public:
	AHealthPickup();
	virtual void Destroyed() override;
protected:
	virtual void OnSphereOverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult
	);

private:
	UPROPERTY(EditAnywhere)
	float HealAmount = 100.f;

	// 서서히 증가
	UPROPERTY(EditAnywhere)
	float HealingTime = 5.f;

	UPROPERTY(VisibleAnywhere)
	class UNiagaraComponent* PickupEffectComponent;

	// 먹을 때 뜰 이펙트
	UPROPERTY(EditAnywhere)
	class UNiagaraSystem* PickupEffect;
};