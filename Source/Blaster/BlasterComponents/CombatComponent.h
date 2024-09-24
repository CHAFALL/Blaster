

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "CombatComponent.generated.h"

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class BLASTER_API UCombatComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UCombatComponent();
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	friend class AMyBlasterCharacter; // friend: 해당 클래스에 전체 액세스 권한을 부여.

	void EquipWeapon(class AWeapon* WeaponToEquip);

protected:
	virtual void BeginPlay() override;

private:
	class AMyBlasterCharacter* Character; // (그냥 자주 쓰여서 뺀거임) 전투 구성 요소는 캐릭터에 접근해야 됨, 따라서 해당 변수 필요. (이러면 캐스팅을 계속 해줄 필요 x)
	AWeapon* EquippedWeapon;

public:	

		
};
