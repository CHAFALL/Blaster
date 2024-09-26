// Fill out your copyright notice in the Description page of Project Settings.


#include "BlasterAnimInstance.h"
#include "MyBlasterCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"

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
	bIsCrouched = MyBlasterCharacter->bIsCrouched;
	bAiming = MyBlasterCharacter->IsAiming();

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
	
}
