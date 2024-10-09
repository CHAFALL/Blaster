

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "BuffComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class BLASTER_API UBuffComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UBuffComponent();
	friend class AMyBlasterCharacter;
	void Heal(float HealAmount, float HealingTime);
	void ReplenishShield(float ShieldAmount, float ReplenishTime);
	void BuffSpeed(float BuffBaseSpeed, float BuffCrouchSpeed, float BuffTime);
	void BuffJump(float BuffJumpVelocity, float BuffTime);
	void SetInitialSpeeds(float BaseSpeed, float CrouchSpeed);
	void SetInitialJumpVelocity(float Velocity);
protected:
	virtual void BeginPlay() override;
	void HealRampUp(float DeltaTime);
	void ShieldRampUp(float DeltaTime);

private:
	UPROPERTY()
	class AMyBlasterCharacter* Character;

	/**
	* Heal buff
	*/

	bool bHealing = false;
	float HealingRate = 0.f;
	float AmountToHeal = 0.f;

	/**
	* Shield buff
	*/

	bool bReplenishingShield = false;
	float ShieldReplenishRate = 0.f;
	float ShieldReplenishAmount = 0.f;

	/**
	* Speed buff
	*/

	FTimerHandle SpeedBuffTimer;
	void ResetSpeeds();
	float InitialBaseSpeed;
	float InitialCrouchSpeed;

	// 멀티플레이어 환경에서는 클라이언트가 개별적으로 값을 계산하는 게 아니라, 
	// 서버가 주도적으로 값(속도)을 결정하고 그 값을 클라이언트들에게 동기화하는 게 중요하다.
	// 이를 위해서는 서버에서 값을 정확하게 복제하고, 클라이언트들이 그 값을 잘 받아서 동기화하도록 하는 것이 필요
	
	// 값 복사 방식에서는 클라이언트들이 서버와는 독립적으로 개별적으로 계산을 수행
	 
	// 서버에서만 중첩 기능이 호출이 되므로 서버와 클라의 속도가 다를 수 있음
	// 서버에서 생각하는 스피드 속도에 따라 서버가 클라를 수정해줘야 됨.
	UFUNCTION(NetMulticast, Reliable)
	void MulticastSpeedBuff(float BaseSpeed, float CrouchSpeed);

	/**
	* Jump buff
	*/

	
	FTimerHandle JumpBuffTimer;
	void ResetJump();
	float InitialJumpVelocity;

	UFUNCTION(NetMulticast, Reliable)
	void MulticastJumpBuff(float JumpVelocity);

public:	
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

		
};
