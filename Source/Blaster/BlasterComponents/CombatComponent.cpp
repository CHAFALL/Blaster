


#include "BlasterComponents/CombatComponent.h"
#include "Blaster/Weapon/Weapon.h"
#include "Blaster/Character/MyBlasterCharacter.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Components/SphereComponent.h"

UCombatComponent::UCombatComponent()
{
	PrimaryComponentTick.bCanEverTick = false; // tick함수에서 무언가를 할 것이라는 것은 실제로 알고 있는 경우에만 하겠다.

}


void UCombatComponent::BeginPlay()
{
	Super::BeginPlay();

	
}


void UCombatComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

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
	
}

