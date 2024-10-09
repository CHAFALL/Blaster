

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PickupSpawnPoint.generated.h"

UCLASS()
class BLASTER_API APickupSpawnPoint : public AActor
{
	GENERATED_BODY()
	
public:	
	APickupSpawnPoint();
	virtual void Tick(float DeltaTime) override;

protected:
	virtual void BeginPlay() override;

	// 여기에다가 우리가 어떤 픽업 종류를 둘 것인지 저장.
	UPROPERTY(EditAnywhere)
	TArray<TSubclassOf<class APickup>> PickupClasses;

	UPROPERTY()
	APickup* SpawnedPickup;

	void SpawnPickup();
	void SpawnPickupTimerFinished();

	UFUNCTION()
	void StartSpawnPickupTimer(AActor* DestroyedActor);

	UPROPERTY(EditAnywhere)
	float SpawnPickupTimeMin;
	
	UPROPERTY(EditAnywhere)
	float SpawnPickupTimeMax;

private:
	FTimerHandle SpawnPickupTimer;
public:	

};


// 주의사항 - 스폰되는 곳에 플레이어가 있다면 해당 스폰지역이 사라지는 문제가 있음.
// -> pickup 쪽에서 overlap되는 시간을 늦춰서 스폰이 될 때 충돌이 발생하지 않게 함