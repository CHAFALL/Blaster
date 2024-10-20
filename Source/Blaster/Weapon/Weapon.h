

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "WeaponTypes.h"
#include "Blaster/BlasterTypes/Team.h" 
#include "Weapon.generated.h"

UENUM(BlueprintType)
enum class EWeaponState : uint8
{
	// 에디터와 청사진에 보여질 이름 설정
	EWS_Initial UMETA(DisplayName = "Initial State"),
	EWS_Equipped UMETA(DisplayName = "Equipped"),
	EWS_EquippedSecondary UMETA(DisplayName = "Equipped Secondary"),
	EWS_Dropped UMETA(DisplayName = "Dropped"),
	// 갯수 파악용
	EWS_MAX UMETA(DisplayName = "DefaultMAX")

};

UENUM(BlueprintType)
enum class EFireType : uint8
{
	EFT_HitScan UMETA(DisplayName = "Hit Scan Weapon"),
	EFT_Projectile UMETA(DisplayName = "Projectile Weapon"),
	EFT_Shotgun UMETA(DisplayName = "Shotgun Weapon"),

	EFT_MAX UMETA(DisplayName = "DefaultMAX")
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
	virtual void Dropped();
	void AddAmmo(int32 AmmoToAdd);
	FVector TraceEndWithScatter(const FVector& HitTarget);



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

	/**
	* Enable or disable custom depth
	*/
	void EnableCustomDepth(bool bEnable);

	bool bDestroyWeapon = false;

	UPROPERTY(EditAnywhere)
	EFireType FireType;

	UPROPERTY(EditAnywhere, Category = "Weapon Scatter")
	bool bUseScatter = false;

protected:
	virtual void BeginPlay() override;
	virtual void OnWeaponStateSet();
	virtual void OnEquipped();
	virtual void OnDropped();
	virtual void OnEquippedSecondary();

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

	/**
	* Trace end with scatter
	*/

	UPROPERTY(EditAnywhere, Category = "Weapon Scatter")
	float DistanceToSphere = 800.f;

	UPROPERTY(EditAnywhere, Category = "Weapon Scatter")
	float SphereRadius = 75.f;

	UPROPERTY(EditAnywhere)
	float Damage = 20.f;

	UPROPERTY(EditAnywhere)
	float HeadShotDamage = 40.f;

	UPROPERTY(Replicated, EditAnywhere)
	bool bUseServerSideRewind = false;

	UPROPERTY()
	class AMyBlasterCharacter* BlasterOwnerCharacter;
	UPROPERTY()
	class ABlasterPlayerController* BlasterOwnerController;

	UFUNCTION()
	void OnPingTooHigh(bool bPingTooHigh);

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
	
	// 값 복사 방식 제거. (클라 측 예측)
	// 클라이언트가 즉시 반응성을 유지하면서도 서버와의 불일치를 최소화하기 위해 시퀀스 번호를 사용해 탄약을 예측하고,
	// 서버의 응답이 돌아오면 그에 맞춰 보정하는 구조로 변경
	UPROPERTY(EditAnywhere)
	int32 Ammo;

	UFUNCTION(Client, Reliable)
	void ClientUpdateAmmo(int32 ServerAmmo);

	UFUNCTION(Client, Reliable)
	void ClientAddAmmo(int32 AmmoToAdd);
	
	void SpendRound();

	UPROPERTY(EditAnywhere)
	int MagCapacity;

	// The number of unprocessed server requests for Ammo.
	// Incremented in SpendRound, decremented in ClientUpdateAmmo.
	int32 Sequence = 0;

	UPROPERTY(EditAnywhere)
	EWeaponType WeaponType;


	// flag 클래스에 배치하는 대신에 무기 클래스에 팀 개념을 주입하는 이유?
	// 무기가 픽업 위젯을 표시하고 캐릭터가 무기와 겹치는 이벤트가 있는 클래스이기 때문에
	// flag인 경우 무기가 동일한 팀을 가지지 않은 한 이러한 일이 발생하지 않도록 방지하기 위해.
	UPROPERTY(EditAnywhere)
	ETeam Team;

public:
	void SetWeaponState(EWeaponState State);
	// 중첩 이벤트는 서버에서만 발동.
	// 서버는 다른 클라가 장착한 무기에 가까이 가도 E 장착 Text가 뜨는 문제가 있는데 그걸 해결하기 위해 구의 충돌을 무력화할 것임.
	FORCEINLINE USphereComponent* GetAreaSphere() const { return AreaSphere; }
	// 무기 철창
	FORCEINLINE USkeletalMeshComponent* GetWeaponMesh() const { return WeaponMesh; }
	FORCEINLINE UWidgetComponent* GetPickupWidget() const { return PickupWidget; }
	FORCEINLINE float GetZoomedFOV() const { return ZoomedFOV; }
	FORCEINLINE float GetZoomInterpSpeed() const { return ZoomInterpSpeed; }
	bool IsEmpty();
	bool IsFull();
	FORCEINLINE EWeaponType GetWeaponType() const { return WeaponType; }
	FORCEINLINE int32 GetAmmo() const { return Ammo; }
	FORCEINLINE int32 GetMagCapacity() const { return MagCapacity; }
	FORCEINLINE float GetDamage() const { return Damage; }
	FORCEINLINE float GetHeadShotDamage() const { return HeadShotDamage; }
	FORCEINLINE ETeam GetTeam() const { return Team; }
};


