

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "CombatComponent.generated.h"

#define TRACE_LENGTH 80000.f;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class BLASTER_API UCombatComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UCombatComponent();
	friend class AMyBlasterCharacter; // friend: 해당 클래스에 전체 액세스 권한을 부여.
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;


	void EquipWeapon(class AWeapon* WeaponToEquip);

protected:
	virtual void BeginPlay() override;
	void SetAiming(bool bIsAiming);

	UFUNCTION(Server, Reliable)
	void ServerSetAiming(bool bIsAiming);

	UFUNCTION()
	void OnRep_EquippedWeapon();

	void FireButtonPressed(bool bPressed);

	UFUNCTION(Server, Reliable)
	void ServerFire(const FVector_NetQuantize& TraceHitTarget ); // FVector를 이용해서 모든 숫자를 정수로 만듬 (직렬화) (많은 정보를 자를 수 있음)

	UFUNCTION(NetMulticast, Reliable)
	void MulticastFire(const FVector_NetQuantize& TraceHitTarget);

	void TraceUnderCrosshairs(FHitResult& TraceHitResult);

	void SetHUDCrosshairs(float DeltaTime);

private:
	class AMyBlasterCharacter* Character; // (그냥 자주 쓰여서 뺀거임) 전투 구성 요소는 캐릭터에 접근해야 됨, 따라서 해당 변수 필요. (이러면 캐스팅을 계속 해줄 필요 x)
	class ABlasterPlayerController* Controller;
	class ABlasterHUD* HUD;

	UPROPERTY(ReplicatedUsing = OnRep_EquippedWeapon)
	AWeapon* EquippedWeapon;

	UPROPERTY(Replicated)
	bool bAiming; // 움크리기나 점프 같은 것은 그냥 지원하지만 이런 것들은 복사를 통해 알려줘야됨.

	UPROPERTY(EditAnywhere)
	float BaseWalkSpeed;

	UPROPERTY(EditAnywhere)
	float AimWalkSpeed;

	bool bFireButtonPressed;

	/**
	* HUD and crosshairs
	*/

	float CrosshairVelocityFactor;
	float CrosshairInAirFactor;

public:	

		
};
