

#include "MyBlasterCharacter.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/WidgetComponent.h"

AMyBlasterCharacter::AMyBlasterCharacter()
{
	PrimaryActorTick.bCanEverTick = true;

	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(GetMesh()); // ĸ���� �ƴ� �׹��濡 ������ ĸ�� ũ�⸦ �ٲ� �� ������ ���� ���̿� ������ ��ġ�� �ʰ� ��.
	CameraBoom->TargetArmLength = 600.f;
	CameraBoom->bUsePawnControlRotation = true; // ���콺�� �Է��� �� ��Ʈ�ѷ��� �Բ� ī�޶� ���� ȸ����ų �� �ְ� ��.

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


