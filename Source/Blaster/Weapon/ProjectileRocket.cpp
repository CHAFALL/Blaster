


#include "ProjectileRocket.h"
#include "Kismet/GameplayStatics.h"

AProjectileRocket::AProjectileRocket()
{
	RocketMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Rocket Mesh"));
	RocketMesh->SetupAttachment(RootComponent);
	RocketMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void AProjectileRocket::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	// Super를 호출하면 destroy가 실행되므로 호출 전에 데미지를 적용하고 싶지만, 로켓은 폭발물임.
	// 따라서 단순히 피해 적용을 다른 액터에 직접 호출하는 대신 방사형 피해를 적용할 것임.
	// 피해를 적용할 때 로켓을 발사한 플레이어의 컨트롤러가 필요
	// 선동자와 소유자가 같다고 생각할 수 있는데 타워디펜스를 예를 들어서 설명하자면
	// 소유자는 타워를 만든 플레이서, 선동자는 타워로 다를 수 있음.(참고)
	APawn* FiringPawn = GetInstigator();
	if (FiringPawn)
	{
		AController* FiringController = FiringPawn->GetController();
		if (FiringController)
		{
			UGameplayStatics::ApplyRadialDamageWithFalloff(
				this, // World context object
				Damage,
				10.f, // 최소한의 데미지 (거리가 먼 경우)
				GetActorLocation(), // Origin
				200.f, // 내부 반지름
				500.f, // 외부 반지름
				1.f, // DamageFalloff
				UDamageType::StaticClass(),
				TArray<AActor*>(), // IgnoreActors 쏜 애도 피해를 입도록.
				this, // DamageCauser
				FiringController // InstigatorController
			);
		}
	}


	Super::OnHit(HitComp, OtherActor, OtherComp, NormalImpulse, Hit);
}
