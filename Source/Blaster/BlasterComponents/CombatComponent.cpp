


#include "BlasterComponents/CombatComponent.h"
#include "Blaster/Weapon/Weapon.h"
#include "Blaster/Character/MyBlasterCharacter.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Components/SphereComponent.h"
#include "Net/UnrealNetwork.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"

UCombatComponent::UCombatComponent()
{
	PrimaryComponentTick.bCanEverTick = true; // tick함수에서 무언가를 할 것이라는 것은 실제로 알고 있는 경우에만 하겠다.

	BaseWalkSpeed = 600.f;
	AimWalkSpeed = 450.f;
}

void UCombatComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UCombatComponent, EquippedWeapon);
	DOREPLIFETIME(UCombatComponent, bAiming);
}

void UCombatComponent::BeginPlay()
{
	Super::BeginPlay();

	if (Character)
	{
		Character->GetCharacterMovement()->MaxWalkSpeed = BaseWalkSpeed;
	}
	
}

// 원래는 bool값을 캐릭터쪽에서 직접 바꿔줬는데 아래 함수를 실행하기 위해 SetAiming을 따로 파줌.
void UCombatComponent::SetAiming(bool bIsAiming)
{
	bAiming = bIsAiming;
	/*if (!Character->HasAuthority())
	{
		ServerSetAiming(bIsAiming);
	}*/ 
	// 그냥 이렇게 해도 됨(RPC 특징을 보면.) (서버만 실행되고 다른데선 어차피 안됨.)
	ServerSetAiming(bIsAiming);

	if (Character)
	{
		Character->GetCharacterMovement()->MaxWalkSpeed = bIsAiming ? AimWalkSpeed : BaseWalkSpeed;
	}

}

// 클라 -> 서버
void UCombatComponent::ServerSetAiming_Implementation(bool bIsAiming)
{
	bAiming = bIsAiming;
	if (Character)
	{
		Character->GetCharacterMovement()->MaxWalkSpeed = bIsAiming ? AimWalkSpeed : BaseWalkSpeed;
	}
}

void UCombatComponent::OnRep_EquippedWeapon()
{
	if (EquippedWeapon && Character)
	{
		Character->GetCharacterMovement()->bOrientRotationToMovement = false;
		Character->bUseControllerRotationYaw = true;
	}
}

void UCombatComponent::FireButtonPressed(bool bPressed)
{
	bFireButtonPressed = bPressed; // 이건 클라이언트의 로컬에 설정해 놓았으므로 여기에 그대로 둠.
	// 이렇게 하면 서버까진 제대로 동작, 클라까지 동작하게 하려고 bFireButtonPressed을 변수 복제하는 것이 쉬울꺼라
	// 생각할 수 있는데 그러면 안됨. (서버가 true로 설정하면 클라도 true가 되잖아!) - 그래도 안되는 이유는 자동화 때문임
	// 점화 버튼을 일정 기간 참조하게 해서 발사하므로 (즉시 반영이 안될 수 있음) , 왜냐? true에서 true는 값 변화가 없어서 반영이 안됨.
	// 멀티 캐스트로 해결하자!!
	if (bFireButtonPressed)
	{
		ServerFire();
	}

}

void UCombatComponent::TraceUnderCrosshairs(FHitResult& TraceHitResult)
{
	FVector2D ViewportSize;
	// 화면 중앙에서
	if (GEngine && GEngine->GameViewport)
	{
		GEngine->GameViewport->GetViewportSize(ViewportSize);
	}

	FVector2D CrosshairLocation(ViewportSize.X / 2.f, ViewportSize.Y / 2.f);

	// 화면에 주어진 정보를 3D 공간 좌표와 방향으로 변환
	FVector CrosshairWorldPosition;
	FVector CrosshairWorldDirection;
	bool bScreenToWorld = UGameplayStatics::DeprojectScreenToWorld(
		UGameplayStatics::GetPlayerController(this, 0),
		CrosshairLocation,
		CrosshairWorldPosition,
		CrosshairWorldDirection
	);

	if (bScreenToWorld)
	{
		FVector Start = CrosshairWorldPosition;

		FVector End = Start + CrosshairWorldDirection * TRACE_LENGTH;

		GetWorld()->LineTraceSingleByChannel(
			TraceHitResult,
			Start,
			End,
			ECollisionChannel::ECC_Visibility
		);
		// 만약에 아무도 부딪히지 않았다면 충돌지점은 End로 처리
		if (!TraceHitResult.bBlockingHit)
		{
			TraceHitResult.ImpactPoint = End;
			HitTarget = End;
		}
		else
		{
			HitTarget = TraceHitResult.ImpactPoint;
			DrawDebugSphere(
				GetWorld(),
				TraceHitResult.ImpactPoint,
				12.f,
				12,
				FColor::Red
			);
		}
	}
}

// 하지만 무기를 발사하는 것과 같은 중요한 건 서버에서 처리
void UCombatComponent::ServerFire_Implementation()
{
	MulticastFire();
}

void UCombatComponent::MulticastFire_Implementation()
{
	if (EquippedWeapon == nullptr) return;
	if (Character)
	{
		Character->PlayFireMontage(bAiming);
		EquippedWeapon->Fire(HitTarget);
	}
}




void UCombatComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	
	FHitResult HitResult;
	TraceUnderCrosshairs(HitResult);
}



void UCombatComponent::EquipWeapon(AWeapon* WeaponToEquip)
{
	if (Character == nullptr || WeaponToEquip == nullptr) return;

	EquippedWeapon = WeaponToEquip;
	EquippedWeapon->SetWeaponState(EWeaponState::EWS_Equipped);
	const USkeletalMeshSocket* HandSocket = Character->GetMesh()->GetSocketByName(FName("RightHandSocket"));
	if (HandSocket)
	{
		HandSocket->AttachActor(EquippedWeapon, Character->GetMesh());
	}

	// 무기 소유자가 캐릭터가 되었으면 함.
	// 우리는 우리가 제어하고 있는 폰을 소유하고 있다고 말하지만, 무기와 같은 액터에는 정의된 소유자가 없음.
	// 하지만 장비를 장착하자마자 장비를 장착한 캐릭터에게 배정해야 됨.
	// Go To Declaration해서 타서 들어가보면 주인이 복제 될 때 신고하는 기능이 있음을 알 수 있음.
	EquippedWeapon->SetOwner(Character);
	Character->GetCharacterMovement()->bOrientRotationToMovement = false;
	Character->bUseControllerRotationYaw = true;
}

