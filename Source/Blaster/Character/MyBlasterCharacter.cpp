

#include "MyBlasterCharacter.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/WidgetComponent.h"
#include "Net/UnrealNetwork.h" // 복제 관련
#include "Blaster/Weapon/Weapon.h"
#include "Blaster/BlasterComponents/CombatComponent.h"
#include "Components/CapsuleComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "BlasterAnimInstance.h"

AMyBlasterCharacter::AMyBlasterCharacter()
{
	PrimaryActorTick.bCanEverTick = true;

	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(GetMesh()); // 캡슐이 아닌 그물방에 부착해 캡슐 크기를 바꿀 때 스프링 암의 높이에 영향을 끼치지 않게 함.
	CameraBoom->TargetArmLength = 600.f;
	CameraBoom->bUsePawnControlRotation = true; // 마우스를 입력할 때 컨트롤러와 함께 카메라 붐을 회전시킬 수 있게 함.

	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	FollowCamera->bUsePawnControlRotation = false;

	bUseControllerRotationYaw = false;
	GetCharacterMovement()->bOrientRotationToMovement = true;

	OverheadWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("OverheadWidget"));
	OverheadWidget->SetupAttachment(RootComponent);

	Combat = CreateDefaultSubobject<UCombatComponent>(TEXT("CombatComponent"));
	// Combat을 통해 우리 캐릭터의 모든 전투 관련 기능을 처리할 것이다. 즉, 전투 구성 요소에 복제될 변수가 있다는 의미!
	Combat->SetIsReplicated(true); // 복제 구성 요소로 지정. 부품은 특별해서 등록이 필요 x

	GetCharacterMovement()->NavAgentProps.bCanCrouch = true;
	// 캡슐이 카메라를 가리지 않도록
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);
	GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);
	GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Visibility, ECollisionResponse::ECR_Block);
	GetCharacterMovement()->RotationRate = FRotator(0.f, 0.f, 850.f); // 회전속도 설정

	TurningInPlace = ETurningInPlace::ETIP_NotTurning;
	// 복제 관련 주기?
	NetUpdateFrequency = 66.f; // 총 게임은 주로 이렇게 씀. 서버 순 틱 비율은 ini파일에서 해야됨. 
	MinNetUpdateFrequency = 33.f;
}

void AMyBlasterCharacter::BeginPlay()
{
	Super::BeginPlay();
	
}

void AMyBlasterCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	AimOffset(DeltaTime);
	HideCameraIfCharacterClose();

}

void AMyBlasterCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ACharacter::Jump);

	PlayerInputComponent->BindAxis("MoveForward", this, &AMyBlasterCharacter::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &AMyBlasterCharacter::MoveRight);
	PlayerInputComponent->BindAxis("Turn", this, &AMyBlasterCharacter::Turn);
	PlayerInputComponent->BindAxis("LookUp", this, &AMyBlasterCharacter::LookUp);

	PlayerInputComponent->BindAction("Equip", IE_Pressed, this, &AMyBlasterCharacter::EquipButtonPressed);
	PlayerInputComponent->BindAction("Crouch", IE_Pressed, this, &AMyBlasterCharacter::CrouchButtonPressed);
	PlayerInputComponent->BindAction("Aim", IE_Pressed, this, &AMyBlasterCharacter::AimButtonPressed);
	PlayerInputComponent->BindAction("Aim", IE_Released, this, &AMyBlasterCharacter::AimButtonReleased);
	PlayerInputComponent->BindAction("Fire", IE_Pressed, this, &AMyBlasterCharacter::FireButtonPressed);
	PlayerInputComponent->BindAction("Fire", IE_Released, this, &AMyBlasterCharacter::FireButtonReleased);

}

void AMyBlasterCharacter::PostInitializeComponents()
{
	Super::PostInitializeComponents();
	if (Combat)
	{
		Combat->Character = this;
	}
}

void AMyBlasterCharacter::PlayFireMontage(bool bAiming)
{
	if (Combat == nullptr || Combat->EquippedWeapon == nullptr) return;

	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && FireWeaponMontage)
	{
		AnimInstance->Montage_Play(FireWeaponMontage);
		// 몽타주 -> 올바른 섹션을 틀어줘야 됨.
		FName SectionName;
		SectionName = bAiming ? FName("RifleAim") : FName("RifleHip");
		AnimInstance->Montage_JumpToSection(SectionName);
	}

}

void AMyBlasterCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	//복제할 OverlappingWeapon를 등록해야 하는 곳
	//DOREPLIFETIME(AMyBlasterCharacter, OverlappingWeapon);
	// 조건을 달아줄 수 있음. (이렇게 하면 서버에서는 볼 수 있지만 클라이언트에서는 해당하는 애만 볼 수 있음.)
	// 알림 호출 위치를 잘 설정하면 서버에서도 안 보임.
	DOREPLIFETIME_CONDITION(AMyBlasterCharacter, OverlappingWeapon, COND_OwnerOnly);
}



void AMyBlasterCharacter::MoveForward(float Value)
{
	if (Controller != nullptr && Value != 0.f)
	{
		const FRotator YawRotation(0.f, Controller->GetControlRotation().Yaw, 0.f);
		const FVector Direction(FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X));
		AddMovementInput(Direction, Value);
	}
}

void AMyBlasterCharacter::MoveRight(float Value)
{
	if (Controller != nullptr && Value != 0.f)
	{
		const FRotator YawRotation(0.f, Controller->GetControlRotation().Yaw, 0.f);
		const FVector Direction(FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y));
		AddMovementInput(Direction, Value);
	}
}

void AMyBlasterCharacter::Turn(float Value)
{
	AddControllerYawInput(Value);
}

void AMyBlasterCharacter::LookUp(float Value)
{
	AddControllerPitchInput(Value);
}

void AMyBlasterCharacter::EquipButtonPressed()
{
	// 누누이 말하지만 무기 관련된 것은 서버에서 다뤄야됨.
	if (Combat)
	{
		if (HasAuthority())
		{
			Combat->EquipWeapon(OverlappingWeapon);
		}
		else
		{
			ServerEquipButtonPressed();
		}

	}

}

void AMyBlasterCharacter::ServerEquipButtonPressed_Implementation()
{
	// 이건 서버에서만 실행되므로 HasAuthority를 할 필요 x
	if (Combat)
	{
		Combat->EquipWeapon(OverlappingWeapon);
	}
}

void AMyBlasterCharacter::CrouchButtonPressed()
{
	if (bIsCrouched)
	{
		UnCrouch();
	}
	else
	{
		Crouch();
	}
}

void AMyBlasterCharacter::AimButtonPressed()
{
	if (Combat)
	{
		Combat->SetAiming(true);
	}
}

void AMyBlasterCharacter::AimButtonReleased()
{
	if (Combat)
	{
		Combat->SetAiming(false);
	}
}

// 무기를 들고 가만히 있을 때만 캐릭터에 영향을 줌.
void AMyBlasterCharacter::AimOffset(float DeltaTime)
{
	if (Combat && Combat->EquippedWeapon == nullptr)
	{
		return;
	}

	FVector Velocity = GetVelocity();
	Velocity.Z = 0.f;
	float Speed = Velocity.Size();
	bool bIsInAir = GetCharacterMovement()->IsFalling();

	if (Speed == 0.f && !bIsInAir) // standing still, not jumping
	{
		FRotator CurrentAimRotation = FRotator(0.f, GetBaseAimRotation().Yaw, 0.f); 
		FRotator DeltaAimRotation = UKismetMathLibrary::NormalizedDeltaRotator(CurrentAimRotation, StartingAimRotation);
		AO_Yaw = DeltaAimRotation.Yaw;
		if (TurningInPlace == ETurningInPlace::ETIP_NotTurning)
		{
			InterpAO_Yaw = AO_Yaw;
		}
		bUseControllerRotationYaw = true;
		TurnInPlace(DeltaTime);

	}
	// 달리다가 갑자기 움직임을 멈추면 앞을 보게 되고 기존 회전 값을 얻게 됨. -> 그래서 처음 값을 저장함.
	if (Speed > 0.f || bIsInAir) // running, or jumping
	{
		StartingAimRotation = FRotator(0.f, GetBaseAimRotation().Yaw, 0.f);
		AO_Yaw = 0.f;
		bUseControllerRotationYaw = true;
		TurningInPlace = ETurningInPlace::ETIP_NotTurning;
	}

	AO_Pitch = GetBaseAimRotation().Pitch;

	if (AO_Pitch > 90.f && !IsLocallyControlled())
	{
		// map pitch from [270, 360) to [-90, 0)
		FVector2D InRange(270.f, 360.f);
		FVector2D OutRange(-90.f, 0.f);
		AO_Pitch = FMath::GetMappedRangeValueClamped(InRange, OutRange, AO_Pitch);
	}

	// Pitch 문제 파악할 때 좋은 로그
	// 0에서 90까지 올라가야 하는데 360에서 270으로 내려감
	// 압축 과정에서 0~360도 식으로 바뀜??(비트마스킹 방식이 빨라서 65536으로 나누는 과정이 있어서?)
	// 직렬화되고 압축되어 네트워크를 통해 전송되고 0~360 사이의 범위로 압축 해제된 후 변경되어 이 범위의 값을 가지게 됨.
	/*if (HasAuthority() && !IsLocallyControlled())
	{
		UE_LOG(LogTemp, Warning, TEXT("AO_Pitch: %f"), AO_Pitch);
	}*/

}

void AMyBlasterCharacter::Jump()
{
	if (bIsCrouched)
	{
		UnCrouch();
	}
	else
	{
		Super::Jump();
	}
}

void AMyBlasterCharacter::FireButtonPressed()
{
	if (Combat)
	{
		Combat->FireButtonPressed(true);
	}
}

void AMyBlasterCharacter::FireButtonReleased()
{
	if (Combat)
	{
		Combat->FireButtonPressed(false);
	}
}

void AMyBlasterCharacter::TurnInPlace(float DeltaTime)
{
	UE_LOG(LogTemp, Warning, TEXT("AO_Yaw: %f"), AO_Yaw);
	if (AO_Yaw > 90.f)
	{
		TurningInPlace = ETurningInPlace::ETIP_Right;
	}
	else if (AO_Yaw < -90.f)
	{
		TurningInPlace = ETurningInPlace::ETIP_Left;
	}
	if (TurningInPlace != ETurningInPlace::ETIP_NotTurning)
	{
		InterpAO_Yaw = FMath::FInterpTo(InterpAO_Yaw, 0.f, DeltaTime, 4.f);
		AO_Yaw = InterpAO_Yaw;
		if (FMath::Abs(AO_Yaw) < 15.f)
		{
			TurningInPlace = ETurningInPlace::ETIP_NotTurning;
			StartingAimRotation = FRotator(0.f, GetBaseAimRotation().Yaw, 0.f);
		}
	}
}

void AMyBlasterCharacter::HideCameraIfCharacterClose()
{
	if (!IsLocallyControlled()) return;
	if ((FollowCamera->GetComponentLocation() - GetActorLocation()).Size() < CameraThreshould)
	{
		GetMesh()->SetVisibility(false);
		if (Combat && Combat->EquippedWeapon && Combat->EquippedWeapon->GetWeaponMesh())
		{
			Combat->EquippedWeapon->GetWeaponMesh()->bOwnerNoSee = true; // 무기 mesh 가시성이 소유자에게만 거짓으로 설정됨.
		}
	}
	else
	{
		GetMesh()->SetVisibility(true);
		if (Combat && Combat->EquippedWeapon && Combat->EquippedWeapon->GetWeaponMesh())
		{
			Combat->EquippedWeapon->GetWeaponMesh()->bOwnerNoSee = false;
		}
	}

}

// 서버는 적용이 안되는 것을 보완하기 위한.
// 클라이언트 측에서 추가 로직을 수행!!!!!!!
// 이렇게 하면 담당자 알림이 서버에서 호출되지 않기 때문에 서버에서 위젯을 가져오지 않음
void AMyBlasterCharacter::SetOverlappingWeapon(AWeapon* Weapon)
{
	if (OverlappingWeapon)
	{
		OverlappingWeapon->ShowPickupWidget(false);
	}

	OverlappingWeapon = Weapon;
	if (IsLocallyControlled()) // 컨트롤 되고 있는 애한테 호출이 되면
	{
		if (OverlappingWeapon)
		{
			OverlappingWeapon->ShowPickupWidget(true);
		}
	}
}




void AMyBlasterCharacter::OnRep_OverlappingWeapon(AWeapon* LastWeapon)
{
	if (OverlappingWeapon)
	{
		OverlappingWeapon->ShowPickupWidget(true);
	}
	if (LastWeapon)
	{
		LastWeapon->ShowPickupWidget(false);
	}
}

bool AMyBlasterCharacter::IsWeaponEquipped()
{
	return (Combat && Combat->EquippedWeapon);
}

bool AMyBlasterCharacter::IsAiming()
{
	return (Combat && Combat->bAiming);
}

AWeapon* AMyBlasterCharacter::GetEquippedWeapon()
{
	if (Combat == nullptr) return nullptr;
	return Combat->EquippedWeapon;
}

FVector AMyBlasterCharacter::GetHitTarget() const
{
	if (Combat == nullptr) return FVector(); // 빈 벡터 반환
	return Combat->HitTarget;
}



