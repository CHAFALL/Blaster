

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "WeaponTypes.h"
#include "Weapon.generated.h"

UENUM(BlueprintType)
enum class EWeaponState : uint8
{
	// 에디터와 청사진에 보여질 이름 설정
	EWS_Initial UMETA(DisplayName = "Initial State"),
	EWS_Equipped UMETA(DisplayName = "Equipped"),
	EWS_Dropped UMETA(DisplayName = "Dropped"),
	// 갯수 파악용
	EWS_MAX UMETA(DisplayName = "DefaultMAX"),

};

UCLASS()
class BLASTER_API AWeapon : public AActor
{
	GENERATED_BODY()
	
public:	
	AWeapon();
	virtual void Tick(float DeltaTime) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	// 클라에서 HUD 탄약을 설정하기 전에 해당 소유자에 액세스해서 컨트롤러를 가져와야 하므로
	// 해당 소유자가 복제되었는지 확인해야 됨.
	// 즉, 현재 우리는 소유자가 복제되기를 기다렸다가  HUD와 무기 상태를 설정할 수 없음..
	// 소유자가 먼저 복제될지 무기 상태가 먼저 될 지 알 수 없음..
	// -> 따라서 클라에서 실제로 HUD 탄약을 설정할 좋은 시점을 알아야 됨.
	virtual void OnRep_Owner() override;
	void SetHUDAmmo();
	void ShowPickupWidget(bool bShowWidget);
	virtual void Fire(const FVector& HitTarget);
	void Dropped();
	void AddAmmo(int32 AmmoToAdd);


	// 십자선은 무기 클래스에 있어야 됨 (무기에 따라 변경되어야 되므로)
	/**
	* Textures for the weapon crosshairs
	*/

	UPROPERTY(EditAnywhere, Category = "Crosshairs")
	class UTexture2D* CrosshairsCenter;

	UPROPERTY(EditAnywhere, Category = "Crosshairs")
	UTexture2D* CrosshairsLeft;

	UPROPERTY(EditAnywhere, Category = "Crosshairs")
	UTexture2D* CrosshairsRight;

	UPROPERTY(EditAnywhere, Category = "Crosshairs")
	UTexture2D* CrosshairsTop;

	UPROPERTY(EditAnywhere, Category = "Crosshairs")
	UTexture2D* CrosshairsBottom;

	/**
	* Zoomed FOV while aiming
	*/

	UPROPERTY(EditAnywhere)
	float ZoomedFOV = 30.f;

	UPROPERTY(EditAnywhere)
	float ZoomInterpSpeed = 20.f;

	/**
	* Automatic fire
	*/

	UPROPERTY(EditAnywhere, Category = Combat)
	float FireDelay = .15f;

	UPROPERTY(EditAnywhere, Category = Combat)
	bool bAutomatic = true;

	UPROPERTY(EditAnywhere)
	class USoundCue* EquipSound;

protected:
	virtual void BeginPlay() override;

	UFUNCTION()
	virtual void OnSphereOverlap(
		UPrimitiveComponent* OverlappedComponent, // 충돌을 감지한 컴포넌트
		AActor* OtherActor, // 현재 객체의 충돌체와 겹친 액터
		UPrimitiveComponent* OtherComp, // 충돌한 객체의 컴포넌트
		int32 OtherBodyIndex, // 충돌한 바디의 인덱스 
		bool bFromSweep,// 충돌이 Sweep으로 발생했는지 여부를 나타냄, true면 객체가 물리적으로 이동하면서 충돌 감지, false면 정적인 상태에서 충돌 발생했음
		const FHitResult& SweepResult // 충돌에 대한 세부 정보가 담긴 구조체
	);

	UFUNCTION()
	void OnSphereEndOverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex
	);

private:	
	UPROPERTY(VisibleAnywhere, Category = "Weapon Properties")
	USkeletalMeshComponent* WeaponMesh;

	// 무기에 닿는 판정을 위해 (캐릭터와 겹치는 부분 감지) - 이부분은 중요해서 서버에서만 하는 게 좋다.
	UPROPERTY(VisibleAnywhere, Category = "Weapon Properties")
	class USphereComponent* AreaSphere;

	UPROPERTY(ReplicatedUsing = OnRep_WeaponState, VisibleAnywhere, Category = "Weapon Properties")
	EWeaponState WeaponState;

	UFUNCTION()
	void OnRep_WeaponState();

	UPROPERTY(VisibleAnywhere, Category = "Weapon Properties")
	class UWidgetComponent* PickupWidget;

	UPROPERTY(EditAnywhere, Category = "Weapon Properties")
	class UAnimationAsset* FireAnimation;

	UPROPERTY(EditAnywhere)
	TSubclassOf<class ACasing> CasingClass;

	UPROPERTY(EditAnywhere, ReplicatedUsing = OnRep_Ammo)
	int32 Ammo;

	UFUNCTION()
	void OnRep_Ammo();
	
	void SpendRound();

	UPROPERTY(EditAnywhere)
	int MagCapacity;

	UPROPERTY()
	class AMyBlasterCharacter* BlasterOwnerCharacter;
	UPROPERTY()
	class ABlasterPlayerController* BlasterOwnerController;

	UPROPERTY(EditAnywhere)
	EWeaponType WeaponType;

public:
	void SetWeaponState(EWeaponState State);
	// 중첩 이벤트는 서버에서만 발동.
	// 서버는 다른 클라가 장착한 무기에 가까이 가도 E 장착 Text가 뜨는 문제가 있는데 그걸 해결하기 위해 구의 충돌을 무력화할 것임.
	FORCEINLINE USphereComponent* GetAreaSphere() const { return AreaSphere; }
	// 무기 철창
	FORCEINLINE USkeletalMeshComponent* GetWeaponMesh() const { return WeaponMesh; }
	FORCEINLINE float GetZoomedFOV() const { return ZoomedFOV; }
	FORCEINLINE float GetZoomInterpSpeed() const { return ZoomInterpSpeed; }
	bool IsEmpty();
	bool IsFull();
	FORCEINLINE EWeaponType GetWeaponType() const { return WeaponType; }
	FORCEINLINE int32 GetAmmo() const { return Ammo; }
	FORCEINLINE int32 GetMagCapacity() const { return MagCapacity; }
};
