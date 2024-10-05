


#include "ProjectileRocket.h"
#include "Kismet/GameplayStatics.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraComponent.h"
#include "Sound/SoundCue.h"
#include "Components/BoxComponent.h"
#include "NiagaraSystemInstanceController.h" // 커뮤
#include "Sound/SoundCue.h"
#include "Components/AudioComponent.h"
#include "RocketMovementComponent.h"

AProjectileRocket::AProjectileRocket()
{
	RocketMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Rocket Mesh"));
	RocketMesh->SetupAttachment(RootComponent);
	RocketMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	RocketMovementComponent = CreateDefaultSubobject<URocketMovementComponent>(TEXT("RocketMovementComponent"));
	RocketMovementComponent->bRotationFollowsVelocity = true;
	RocketMovementComponent->SetIsReplicated(true);
}


void AProjectileRocket::BeginPlay()
{
	Super::BeginPlay();

	// 특이
	if (!HasAuthority())
	{
		CollisionBox->OnComponentHit.AddDynamic(this, &AProjectileRocket::OnHit);
	}

	if (TrailSystem)
	{
		TrailSystemComponent = UNiagaraFunctionLibrary::SpawnSystemAttached(
			TrailSystem,
			GetRootComponent(),
			FName(),
			GetActorLocation(),
			GetActorRotation(),
			EAttachLocation::KeepWorldPosition,
			false // auto destroy
		);
	}
	if (ProjectileLoop && LoopingSoundAttenuation)
	{
		ProjectileLoopComponent = UGameplayStatics::SpawnSoundAttached(
			ProjectileLoop,
			GetRootComponent(),
			FName(),
			GetActorLocation(),
			EAttachLocation::KeepWorldPosition,
			false,
			1.f, // 볼륨 증폭기
			1.f,
			0.f,
			LoopingSoundAttenuation,
			(USoundConcurrency*)nullptr,
			false
		);
	}

	// 위에서 단 소리는 로켓이 파괴될 때가 아니라 로켓이 명중할 때 멈추게 하고픔
}

void AProjectileRocket::DestroyTimerFinished()
{
	Destroy(); // 파괴 신호를 3초 늦춤
}


void AProjectileRocket::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	if (OtherActor == GetOwner()) // 버그를 막기 위한. - 해당 조건에 그냥 return을 때리면 그냥 로켓이 비행을 멈추는 증상이 생김.
	{
		UE_LOG(LogTemp, Warning, TEXT("Hit self"));
		return; 
	}

	// 이를 해결하기 위해 로켓이 명중해도 더 이상 이동을 멈추지 않는 로켓 전용 발사체 이동 구성 요소를 만들자!
	// 또 다른 방법으로 로켓 발가시의 소켓을 더 앞으로 이동시켜서 로켓을 발사할 때 로켓 통에서 멀리 떨어지게 해서 해결하는 사람도 있음.

	APawn* FiringPawn = GetInstigator();
	if (FiringPawn && HasAuthority())
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

	GetWorldTimerManager().SetTimer(
		DestroyTimer,
		this,
		&AProjectileRocket::DestroyTimerFinished,
		DestroyTime
	);

	if (ImpactParticles)
	{
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ImpactParticles, GetActorTransform());
	}

	if (ImpactSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, ImpactSound, GetActorLocation());
	}
	if (RocketMesh)
	{
		RocketMesh->SetVisibility(false);
	}
	if (CollisionBox)
	{
		CollisionBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}
	// 생성 막기 - 커뮤
	if (TrailSystemComponent && TrailSystemComponent->GetSystemInstanceController())
	{
		TrailSystemComponent->GetSystemInstanceController()->Deactivate();
	}
	if (ProjectileLoopComponent && ProjectileLoopComponent->IsPlaying())
	{
		ProjectileLoopComponent->Stop();
	}

	//Super::OnHit(HitComp, OtherActor, OtherComp, NormalImpulse, Hit); // 바로 파괴시키지 않겠다. (타이머로 시간 되면 파괴)
	// 이걸로 안하니 Destroyed도 오버라이드 해서 설정해줘야 됨.
}

void AProjectileRocket::Destroyed()
{
}
