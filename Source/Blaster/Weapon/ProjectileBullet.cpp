


#include "Weapon/ProjectileBullet.h"
#include "Kismet/GameplayStatics.h"
#include "Blaster/Character/MyBlasterCharacter.h"
#include "Blaster/PlayerController/BlasterPlayerController.h"
#include "Blaster/BlasterComponents/LagCompensationComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"


AProjectileBullet::AProjectileBullet()
{
	ProjectileMovementComponent = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovementComponent"));
	// 속도에 맞춰 회전을 유지가능 (따라서 중력으로 인한 경로 구성의 회전이 해당 궤적을 따름)
	ProjectileMovementComponent->bRotationFollowsVelocity = true;
	ProjectileMovementComponent->SetIsReplicated(true);
	ProjectileMovementComponent->InitialSpeed = InitialSpeed;
	ProjectileMovementComponent->MaxSpeed = InitialSpeed;
}
#if WITH_EDITOR
void AProjectileBullet::PostEditChangeProperty(FPropertyChangedEvent& Event)
{
	Super::PostEditChangeProperty(Event);

	FName PropertyName = Event.Property != nullptr ? Event.Property->GetFName() : NAME_None;
	if (PropertyName == GET_MEMBER_NAME_CHECKED(AProjectileBullet, InitialSpeed))
	{
		if (ProjectileMovementComponent)
		{
			ProjectileMovementComponent->InitialSpeed = InitialSpeed;
			ProjectileMovementComponent->MaxSpeed = InitialSpeed;
		}
	}
}
#endif

void AProjectileBullet::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	AMyBlasterCharacter* OwnerCharacter = Cast<AMyBlasterCharacter>(GetOwner());
	if (OwnerCharacter)
	{
		ABlasterPlayerController* OwnerController = Cast<ABlasterPlayerController>(OwnerCharacter->Controller);
		if (OwnerController)
		{
			if (OwnerCharacter->HasAuthority() && !bUseServerSideRewind)
			{
				const float DamageToCause = Hit.BoneName.ToString() == FString("head") ? HeadShotDamage : Damage;

				UGameplayStatics::ApplyDamage(OtherActor, DamageToCause, OwnerController, this, UDamageType::StaticClass());
				Super::OnHit(HitComp, OtherActor, OtherComp, NormalImpulse, Hit); // 해당 버전을 들어가면 발사체를 destroy 함..
				return;
			}
			AMyBlasterCharacter* HitCharacter = Cast<AMyBlasterCharacter>(OtherActor);
			if (bUseServerSideRewind && OwnerCharacter->GetLagCompensation() && OwnerCharacter->IsLocallyControlled() && HitCharacter)
			{
				OwnerCharacter->GetLagCompensation()->ProjectileServerScoreRequest(
					HitCharacter,
					TraceStart,
					InitialVelocity,
					OwnerController->GetServerTime() - OwnerController->SingleTripTime
				);
			}

		}
	}


	Super::OnHit(HitComp, OtherActor, OtherComp, NormalImpulse, Hit); // 해당 버전을 들어가면 발사체를 destroy 함..
}

void AProjectileBullet::BeginPlay()
{
	Super::BeginPlay();

	
	//FPredictProjectilePathParams PathParams;
	//PathParams.bTraceWithChannel = true;
	//PathParams.bTraceWithCollision = true;
	//PathParams.DrawDebugTime = 5.f;
	//PathParams.DrawDebugType = EDrawDebugTrace::ForDuration;
	//PathParams.LaunchVelocity = GetActorForwardVector() * InitialSpeed;
	//PathParams.MaxSimTime = 4.f; //발사체의 궤적을 시뮬레이션할 최대 시간을 설정
	//PathParams.ProjectileRadius = 5.f;
	//PathParams.SimFrequency = 30.f; // 시뮬레이션 빈도를 설정
	//PathParams.StartLocation = GetActorLocation();
	//PathParams.TraceChannel = ECollisionChannel::ECC_Visibility;
	//PathParams.ActorsToIgnore.Add(this);

	//FPredictProjectilePathResult PathResult;

	//UGameplayStatics::PredictProjectilePath(this, PathParams, PathResult);
}
