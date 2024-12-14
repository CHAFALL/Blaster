


#include "HitScanWeapon.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Blaster/Character/MyBlasterCharacter.h"
#include "Blaster/PlayerController/BlasterPlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystemComponent.h"
#include "Sound/SoundCue.h"
#include "WeaponTypes.h"
#include "Blaster/BlasterComponents/LagCompensationComponent.h"

void AHitScanWeapon::Fire(const FVector& HitTarget)
{
	Super::Fire(HitTarget);

	APawn* OwnerPawn = Cast<APawn>(GetOwner());
	if (OwnerPawn == nullptr) return;
	AController* InstigatorController = OwnerPawn->GetController();

	// InstigatorController는 시뮬레이션 프록시에선 유효하지 않음을 알 수 있음.
	// 그래서 총알이 날아가는 궤도가 안보인 것임.
	if (!HasAuthority() && InstigatorController) UE_LOG(LogTemp, Warning, TEXT("Instigator valid"))

	const USkeletalMeshSocket* MuzzleFlashSocket = GetWeaponMesh()->GetSocketByName("MuzzleFlash");
	if (MuzzleFlashSocket)
	{
		FTransform SocketTransform = MuzzleFlashSocket->GetSocketTransform(GetWeaponMesh());
		FVector Start = SocketTransform.GetLocation();

		FHitResult FireHit;
		WeaponTraceHit(Start, HitTarget, FireHit);

		AMyBlasterCharacter* BlasterCharacter = Cast<AMyBlasterCharacter>(FireHit.GetActor());
		if (BlasterCharacter && InstigatorController)
		{
			bool bCauseAuthDamage = !bUseServerSideRewind || OwnerPawn->IsLocallyControlled();
			if (HasAuthority() && bCauseAuthDamage)
			{
				const float DamageToCause = FireHit.BoneName.ToString() == FString("head") ? HeadShotDamage : Damage;

				UGameplayStatics::ApplyDamage(
					BlasterCharacter,
					DamageToCause,
					InstigatorController,
					this,
					UDamageType::StaticClass()
				);
			}
			// 서버가 아니면
			// 동기화 시간 필요. 즉, 플레이어 컨트롤러에 액세스해야함.
			if (!HasAuthority() && bUseServerSideRewind)
			{
				BlasterOwnerCharacter = BlasterOwnerCharacter == nullptr ? Cast<AMyBlasterCharacter>(OwnerPawn) : BlasterOwnerCharacter;
				BlasterOwnerController = BlasterOwnerController == nullptr ? Cast<ABlasterPlayerController>(InstigatorController) : BlasterOwnerController;
				if (BlasterOwnerController && BlasterOwnerCharacter && BlasterOwnerCharacter->GetLagCompensation() && BlasterOwnerCharacter->IsLocallyControlled())
				{
					BlasterOwnerCharacter->GetLagCompensation()->ServerScoreRequest(
						BlasterCharacter,
						Start,
						HitTarget,
						BlasterOwnerController->GetServerTime() - BlasterOwnerController->SingleTripTime // 클라이언트가 특정한 패킷을 서버에 보냈던 시점의 서버 시간을 추정
					);
				}
			}
			
		}
		if (ImpactParticles)
		{
			UGameplayStatics::SpawnEmitterAtLocation(
				GetWorld(),
				ImpactParticles,
				FireHit.ImpactPoint,
				FireHit.ImpactNormal.Rotation()
			);
		}
		if (HitSound)
		{
			UGameplayStatics::PlaySoundAtLocation(
				this,
				HitSound,
				FireHit.ImpactPoint
			);
		}

		if (MuzzleFlash)
		{
			UGameplayStatics::SpawnEmitterAtLocation(
				GetWorld(),
				MuzzleFlash,
				SocketTransform
			);
		}
		if (FireSound)
		{
			UGameplayStatics::PlaySoundAtLocation(
				this,
				FireSound,
				GetActorLocation()
			);
		}
	}
}

// 여러번 호출할 것이기 때문에 피해적용을 여러번 호출하는 대신에 적중 횟수를 집계하고 피해를 합산한
// 다음 피해 적용을 한 번 호출하는 것이 더 효율적이다!
void AHitScanWeapon::WeaponTraceHit(const FVector& TraceStart, const FVector& HitTarget, FHitResult& OutHit)
{
	UWorld* World = GetWorld();
	if (World)
	{
		// 서버에서 분산을 계산하는 것이 아닌 로컬에서 분산을 계산한 뒤에 그 결과값을 넘겨줄 것임. (이러면 지연시간도 줄일 수 있고, 서버와 클라의 분산된 것의 차이를 없앨 수 있음)
		// (기존엔 로컬 분산과 서버 분산을 둘 다 하고 판정은 서버 것으로 했음.)
		// 로컬에서 분산할 것이므로 무기 추적 표적을 확인할 필요 x (TraceEndWithScatter 부분을 뺌.)
		FVector End = TraceStart + (HitTarget - TraceStart) * 1.25f;
		World->LineTraceSingleByChannel(
			OutHit,
			TraceStart,
			End,
			ECollisionChannel::ECC_Visibility
		);
		FVector BeamEnd = End;
		if (OutHit.bBlockingHit)
		{
			BeamEnd = OutHit.ImpactPoint;
		}
		else
		{
			OutHit.ImpactPoint = End;
		}

		DrawDebugSphere(GetWorld(), BeamEnd, 16.f, 12, FColor::Orange, true);

		if (BeamParticles)
		{
			UParticleSystemComponent* Beam = UGameplayStatics::SpawnEmitterAtLocation(
				World,
				BeamParticles,
				TraceStart,
				FRotator::ZeroRotator,
				true
			);
			if (Beam)
			{
				Beam->SetVectorParameter(FName("Target"), BeamEnd);
			}
		}
	}
}





