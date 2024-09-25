// Fill out your copyright notice in the Description page of Project Settings.


#include "BlasterAnimInstance.h"
#include "MyBlasterCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"

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
	
}
