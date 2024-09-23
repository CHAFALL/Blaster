

#include "MyBlasterCharacter.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/WidgetComponent.h"

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

void AMyBlasterCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ACharacter::Jump);

	PlayerInputComponent->BindAxis("MoveForward", this, &AMyBlasterCharacter::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &AMyBlasterCharacter::MoveRight);
	PlayerInputComponent->BindAxis("Turn", this, &AMyBlasterCharacter::Turn);
	PlayerInputComponent->BindAxis("LookUp", this, &AMyBlasterCharacter::LookUp);

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

void AMyBlasterCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}


