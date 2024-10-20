// Fill out your copyright notice in the Description page of Project Settings.


#include "BlasterAnimInstance.h"
#include "MyBlasterCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Blaster/Weapon/Weapon.h"
#include "Blaster/BlasterTypes/CombatState.h"

void UBlasterAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();

	MyBlasterCharacter = Cast<AMyBlasterCharacter>(TryGetPawnOwner());
}

void UBlasterAnimInstance::NativeUpdateAnimation(float DeltaTime)
{
	Super::NativeUpdateAnimation(DeltaTime);

	if (MyBlasterCharacter == nullptr)
	{
		MyBlasterCharacter = Cast<AMyBlasterCharacter>(TryGetPawnOwner());
	}

	if (MyBlasterCharacter == nullptr) return;

	FVector Velocity = MyBlasterCharacter->GetVelocity();
	Velocity.Z = 0.f;
	Speed = Velocity.Size();

	bIsInAir = MyBlasterCharacter->GetCharacterMovement()->IsFalling();
	bIsAccelerating = MyBlasterCharacter->GetCharacterMovement()->GetCurrentAcceleration().Size() > 0.f ? true : false;
	bWeaponEquipped = MyBlasterCharacter->IsWeaponEquipped();
	EquippedWeapon = MyBlasterCharacter->GetEquippedWeapon();
	bIsCrouched = MyBlasterCharacter->bIsCrouched;
	bAiming = MyBlasterCharacter->IsAiming();
	TurningInPlace = MyBlasterCharacter->GetTurningInPlace();
	bRotateRootBone = MyBlasterCharacter->ShouldRotateRootBone();
	bElimmed = MyBlasterCharacter->IsElimmed();
	bHoldingTheFlag = MyBlasterCharacter->IsHoldingTheFlag();

	// 글로벌 기준임. (오른쪽으로 회전하면 180도까지 증가하다가 초과하면 -180에서부터 0으로)
	// Offset Yaw for Strafing
	FRotator AimRotation = MyBlasterCharacter->GetBaseAimRotation();
	FRotator MovementRotation = UKismetMathLibrary::MakeRotFromX(MyBlasterCharacter->GetVelocity());
	FRotator DeltaRot = UKismetMathLibrary::NormalizedDeltaRotator(MovementRotation, AimRotation);
	DeltaRotation = FMath::RInterpTo(DeltaRotation, DeltaRot, DeltaTime, 6.f);
	YawOffset = DeltaRotation.Yaw;

	CharacterRotationLastFrame = CharacterRotation;
	CharacterRotation = MyBlasterCharacter->GetActorRotation();
	const FRotator Delta = UKismetMathLibrary::NormalizedDeltaRotator(CharacterRotation, CharacterRotationLastFrame);
	const float Target = Delta.Yaw / DeltaTime;
	// FInterpTo 함수는 Lean 값을 Target 값에 점차적으로 가까워지도록 보간합니다.
	const float Interp = FMath::FInterpTo(Lean, Target, DeltaTime, 6.f);
	Lean = FMath::Clamp(Interp, -90.f, 90.f);

	AO_Yaw = MyBlasterCharacter->GetAO_Yaw();
	AO_Pitch = MyBlasterCharacter->GetAO_Pitch();

	// 왼쪽에서 오른쪽으로 이동할 때 에니메이션이 확 바뀌어서  부드러운 보간을 원한다.
	// 그래서 적용시키니 다른 곳이 부작용이 발생....
	// 그럼 어떻게??
	// 오프셋을 가져와서 180에서 -180으로 확 바뀌는 것을 변경할 예정! (DeltaRot)
	if (bWeaponEquipped && EquippedWeapon && EquippedWeapon->GetWeaponMesh() && MyBlasterCharacter->GetMesh())
	{
		LeftHandTransform = EquippedWeapon->GetWeaponMesh()->GetSocketTransform(FName("LeftHandSocket"), ERelativeTransformSpace::RTS_World);
		FVector OutPosition;
		FRotator OutRotation;
		MyBlasterCharacter->GetMesh()->TransformToBoneSpace(FName("hand_r"), LeftHandTransform.GetLocation(), FRotator::ZeroRotator, OutPosition, OutRotation);
		LeftHandTransform.SetLocation(OutPosition);
		LeftHandTransform.SetRotation(FQuat(OutRotation));


		if (MyBlasterCharacter->IsLocallyControlled())
		{
			bLocallyControlled = true;
			/*FTransform RightHandTransform = EquippedWeapon->GetWeaponMesh()->GetSocketTransform(FName("Hand_R"), ERelativeTransformSpace::RTS_World);*/
			FTransform RightHandTransform = MyBlasterCharacter->GetMesh()->GetSocketTransform(FName("hand_r"), ERelativeTransformSpace::RTS_World); // 커뮤 방식

			// 오른쪽 뼈 위치에서 적중 대상까지의 회전을 살펴보자! (반대 방향으로 되어있어서 한번 더 뒤집음.)
			FRotator LookAtRotation = UKismetMathLibrary::FindLookAtRotation(RightHandTransform.GetLocation(), RightHandTransform.GetLocation() + (RightHandTransform.GetLocation() - MyBlasterCharacter->GetHitTarget()));
			RightHandRotation = FMath::RInterpTo(RightHandRotation, LookAtRotation, DeltaTime, 30.f);
		}
	}

	bUseFABRIK = MyBlasterCharacter->GetCombatState() == ECombatState::ECS_Unoccupied;
	bool bFABRIKOverride = MyBlasterCharacter->IsLocallyControlled() &&
		MyBlasterCharacter->GetCombatState() != ECombatState::ECS_ThrowingGrenade &&
		MyBlasterCharacter->bFinishedSwapping;
	if (bFABRIKOverride)
	{
		bUseFABRIK = !MyBlasterCharacter->IsLocallyReloading(); // 로컬인 경우 위의 것을 무시하고 판단
	}
	bUseAimOffsets = MyBlasterCharacter->GetCombatState() == ECombatState::ECS_Unoccupied && !MyBlasterCharacter->GetDisableGameplay();
	bTransformRightHand = MyBlasterCharacter->GetCombatState() == ECombatState::ECS_Unoccupied && !MyBlasterCharacter->GetDisableGameplay();
}

