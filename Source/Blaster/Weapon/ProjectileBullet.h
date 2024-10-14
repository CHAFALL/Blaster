

#pragma once

#include "CoreMinimal.h"
#include "Projectile.h"
#include "ProjectileBullet.generated.h"

/**
 * 
 */
UCLASS()
class BLASTER_API AProjectileBullet : public AProjectile
{
	GENERATED_BODY()
	
public:
	AProjectileBullet();

	// 청사진 변수는 C++ 변수를 그냥 재정의 해버림...
	// 언리얼 엔진에서 에디터에서 변경된 속성 값에 따라 InitialSpeed와 MaxSpeed를 실시간으로 업데이트하고 싶음
	// 조건부 컴파일 (런타임이 아닌 컴파일 타임에 확인됨.)
#if WITH_EDITOR
	// 언리얼 에디터에서 액터의 속성이 변경될 때 호출되는 함수
	virtual void PostEditChangeProperty(struct FPropertyChangedEvent& Event) override;
#endif

protected:
	virtual void OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit) override;
	virtual void BeginPlay() override;
};
