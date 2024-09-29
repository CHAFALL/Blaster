// Fill out your copyright notice in the Description page of Project Settings.


#include "BlasterAnimInstance.h"
#include "MyBlasterCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Blaster/Weapon/Weapon.h"

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

	// anim 인스턴스에서 이 값을 계산해주는 것을 볼 수 있다. (복제에 대한 걱정이 없어도 됨.)
	// 기본 목표 회전은 클라와 서버에 실제로 설정되어있고 이미 복제된 값을 사용하기 때문임.

	// 어떤 변수가 이미 복제됐는지 더 많이 알수록 더 많은 설정을 할 수 있다.
	// 네트워크로 더 많은 데이터를 보내지 않아도 되고 게임 중 더 많은 대역폭을 쓰지 않아도 된다.

	// 왼쪽에서 오른쪽으로 이동할 때 에니메이션이 확 바뀌어서  부드러운 보간을 원한다.
	// 그래서 적용시키니 다른 곳이 부작용이 발생....
	// 그럼 어떻게??
	// 오프셋을 가져와서 180에서 -180으로 확 바뀌는 것을 변경할 예정! (DeltaRot)
	if (bWeaponEquipped && EquippedWeapon && EquippedWeapon->GetWeaponMesh() && MyBlasterCharacter->GetMesh())
	{
		// 이러면 왼손이 세계 공간에 맞춰서 변형됨.
		LeftHandTransform = EquippedWeapon->GetWeaponMesh()->GetSocketTransform(FName("LeftHandSocket"), ERelativeTransformSpace::RTS_World);
		// 흉기의 소켓 위치를 오른손에 맞춰야 됨. (무기를 런타임 동안 오른손과 상대적으로 움직이면 안 되기 때문.)
		// 우리 오른손은 뼈의 공간에서 기준이 되는 뼈대이다.
		// TransformToBoneSpace 함수를 사용하여,
		// 월드 좌표계를 특정 본(Bone) 좌표계로 변환하는 작업을 수행하는 코드입니다.
		// 즉, 월드 공간에 있는 위치와 회전을 특정 본(여기서는 hand_r 본)을 기준으로 변환합니다.
		// 이렇게 함으로써, 월드 공간의 특정 지점을 본 좌표계로 표현할 수 있게 됩니다.
		// 변환된 위치, 회전 정보
		FVector OutPosition;
		FRotator OutRotation;
		MyBlasterCharacter->GetMesh()->TransformToBoneSpace(FName("hand_r"), LeftHandTransform.GetLocation(), FRotator::ZeroRotator, OutPosition, OutRotation);
		// 뼈 공간에서 정확한 위치와 회전을 찾았으니 왼쪽 회전위치를 설정할 수 있음
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

		// 어긋난 정도를 분석하기 위함
		//FTransform MuzzleTipTransform = EquippedWeapon->GetWeaponMesh()->GetSocketTransform(FName("MuzzleFlash"), ERelativeTransformSpace::RTS_World);
		//FVector MuzzleX(FRotationMatrix(MuzzleTipTransform.GetRotation().Rotator()).GetUnitAxis(EAxis::X));
		//DrawDebugLine(GetWorld(), MuzzleTipTransform.GetLocation(), MuzzleTipTransform.GetLocation() + MuzzleX * 1000.f, FColor::Red); // 총구 끝에서 직선으로
		//DrawDebugLine(GetWorld(), MuzzleTipTransform.GetLocation(), MyBlasterCharacter->GetHitTarget(), FColor::Orange); // 총구 끝에서 조준선으로 가리킨 적중 대상까지

	}
}

