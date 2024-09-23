

#include "MyBlasterCharacter.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/WidgetComponent.h"
#include "Net/UnrealNetwork.h" // 복제 관련
#include "Blaster/Weapon/Weapon.h"

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
}

void AMyBlasterCharacter::BeginPlay()
{
	Super::BeginPlay();
	
}

void AMyBlasterCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}


void AMyBlasterCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ACharacter::Jump);

	PlayerInputComponent->BindAxis("MoveForward", this, &AMyBlasterCharacter::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &AMyBlasterCharacter::MoveRight);
	PlayerInputComponent->BindAxis("Turn", this, &AMyBlasterCharacter::Turn);
	PlayerInputComponent->BindAxis("LookUp", this, &AMyBlasterCharacter::LookUp);

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




