


#include "BlasterComponents/CombatComponent.h"
#include "Blaster/Weapon/Weapon.h"
#include "Blaster/Character/MyBlasterCharacter.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Components/SphereComponent.h"
#include "Net/UnrealNetwork.h"
#include "GameFramework/CharacterMovementComponent.h"

UCombatComponent::UCombatComponent()
{
	PrimaryComponentTick.bCanEverTick = false; // tick함수에서 무언가를 할 것이라는 것은 실제로 알고 있는 경우에만 하겠다.

	BaseWalkSpeed = 600.f;
	AimWalkSpeed = 450.f;
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
	bFireButtonPressed = bPressed;

	if (EquippedWeapon == nullptr) return;
	if (Character && bFireButtonPressed)
	{
		Character->PlayFireMontage(bAiming);
		EquippedWeapon->Fire();
	}
}


void UCombatComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

}

void UCombatComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UCombatComponent, EquippedWeapon);
	DOREPLIFETIME(UCombatComponent, bAiming);
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

