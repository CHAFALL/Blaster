

#pragma once

#include "CoreMinimal.h"
#include "Weapon.h"
#include "Flag.generated.h"

/**
 * 
 */
UCLASS()
class BLASTER_API AFlag : public AWeapon
{
	GENERATED_BODY()

public:
	AFlag();
	virtual void Dropped() override;

protected:
	virtual void OnEquipped() override;
	virtual void OnDropped() override;
private:

	// 깃발은 정적메쉬로 할 것.
	UPROPERTY(VisibleAnywhere)
	UStaticMeshComponent* FlagMesh;
	
};
