


#include "Weapon/Weapon.h"
#include "Components/SphereComponent.h"
#include "Components/WidgetComponent.h"
#include "Blaster/Character/MyBlasterCharacter.h"
#include "Net/UnrealNetwork.h"
#include "Animation/AnimationAsset.h"
#include "Components/SkeletalMeshComponent.h"
#include "Casing.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Blaster/PlayerController/BlasterPlayerController.h"

// Sets default values
AWeapon::AWeapon()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false; // 나중에 무기를 작동시켜야 한다고 결정하면 언제든 변경 가능.
	bReplicates = true;

	WeaponMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("WeaponMesh"));
	SetRootComponent(WeaponMesh);

	// 나중에 무기를 떨굴것인데 떨굴 때마다 땅에 튕겨서 벽에 닿았으면 함. -> 따라서 모든 채널에 대한 충돌 응답을 차단하고 Pawn에 대한 것은 ignore로
	WeaponMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Block);
	WeaponMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Ignore);
	// 무기는 그냥 가서 집을 수 있는 상태에서 시작할 것이다.
	// 따라서 시작엔 무기 메쉬가 충돌을 비활성화하도록 하고 싶음.
	WeaponMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	AreaSphere = CreateDefaultSubobject<USphereComponent>(TEXT("AreaSphere"));
	AreaSphere->SetupAttachment(RootComponent);
	// 서버에 있을 때만 구체가 overlapped 현상을 감지하게 하고 싶음. -> 권한 판별은 BeginPlay에서
	AreaSphere->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	AreaSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	PickupWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("PickupWidget"));
	PickupWidget->SetupAttachment(RootComponent);
}

void AWeapon::BeginPlay()
{
	Super::BeginPlay();
	
	//if (GetLocalRole() == ENetRole::ROLE_Authority) 같은 것임.
	// 모든 무기류는 서버가 관리 - 이렇게만 하면 서버만 동작하니깐 변수 복제를 이용해서 클라에도 적용시키자!!
	if (HasAuthority())
	{
		AreaSphere->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		AreaSphere->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Overlap);
		// 델리게이트와 관련이 되어있는데, 델리게이트는 특정한 매개변수를 전달하도록 설계되어있음. (해당하는 객체의 인스턴스에서만 실행이 되도록 하려고)
		AreaSphere->OnComponentBeginOverlap.AddDynamic(this, &AWeapon::OnSphereOverlap); // 첫번째 인자는 사용자 객체이고 두번째 인자는 콜백 함수. 
		AreaSphere->OnComponentEndOverlap.AddDynamic(this, &AWeapon::OnSphereEndOverlap);
	}
	
	if (PickupWidget)
	{
		PickupWidget->SetVisibility(false);
	}
}


void AWeapon::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AWeapon::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AWeapon, WeaponState); 
	DOREPLIFETIME(AWeapon, Ammo);
}

void AWeapon::OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	// 무기에 가까이 갔을 때만 E를 눌러 장착하라는 Text가 보이도록
	AMyBlasterCharacter* BlasterCharacter = Cast<AMyBlasterCharacter>(OtherActor);
	if (BlasterCharacter)
	{
		BlasterCharacter->SetOverlappingWeapon(this);
	}
}

void AWeapon::OnSphereEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	AMyBlasterCharacter* BlasterCharacter = Cast<AMyBlasterCharacter>(OtherActor);
	if (BlasterCharacter)
	{
		BlasterCharacter->SetOverlappingWeapon(nullptr);
	}
}

void AWeapon::SetHUDAmmo()
{
	BlasterOwnerCharacter = BlasterOwnerCharacter == nullptr ? Cast<AMyBlasterCharacter>(GetOwner()) : BlasterOwnerCharacter;
	if (BlasterOwnerCharacter)
	{
		BlasterOwnerController = BlasterOwnerController == nullptr ? Cast<ABlasterPlayerController>(BlasterOwnerCharacter->Controller) : BlasterOwnerController;
		if (BlasterOwnerController)
		{
			BlasterOwnerController->SetHUDWeaponAmmo(Ammo);
		}
	}
}


// 매우 자주 호출 -> 그래서 변수 저장. (BlasterOwnerCharacter, BlasterOwnerController)
void AWeapon::SpendRound() // 총을 쏠 때 시간을 보내야 됨.
{
	Ammo = FMath::Clamp(Ammo - 1, 0, MagCapacity);
	SetHUDAmmo();
}

void AWeapon::OnRep_Ammo()
{
	SetHUDAmmo();
}

void AWeapon::OnRep_Owner()
{
	Super::OnRep_Owner();
	if (Owner == nullptr)
	{
		BlasterOwnerCharacter = nullptr; 
		BlasterOwnerController = nullptr; 
	}
	else
	{
		SetHUDAmmo();
	}
}


void AWeapon::SetWeaponState(EWeaponState State)
{
	WeaponState = State;
	switch (WeaponState)
	{
	case EWeaponState::EWS_Equipped:
		ShowPickupWidget(false);
		AreaSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		WeaponMesh->SetSimulatePhysics(false); 
		WeaponMesh->SetEnableGravity(false);
		WeaponMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		break;
	case EWeaponState::EWS_Dropped:
		if (HasAuthority())
		{
			AreaSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
		}
		WeaponMesh->SetSimulatePhysics(true); // 땅에 닿고 튕겨나오게 하려고
		WeaponMesh->SetEnableGravity(true);
		WeaponMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		// 누군가가 무기를 장착하려 하면 손의 소켓에 부착이 될텐데 물리학이 여전히 활성화되어 있으면 작동 x
		// 따라서 무기를 장착하기 전에 장착된 무기 상태 설정을 호출하고 여기에서 물리 및 충돌 비활성화를 다시 처리해줘야 됨.
		break;
	}
}


void AWeapon::OnRep_WeaponState()
{
	switch (WeaponState)
	{
	case EWeaponState::EWS_Equipped:
		ShowPickupWidget(false);
		WeaponMesh->SetSimulatePhysics(false);
		WeaponMesh->SetEnableGravity(false);
		WeaponMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		break;
	case EWeaponState::EWS_Dropped:
		WeaponMesh->SetSimulatePhysics(true); // 땅에 닿고 튕겨나오게 하려고
		WeaponMesh->SetEnableGravity(true);
		WeaponMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		// 누군가가 무기를 장착하려 하면 손의 소켓에 부착이 될텐데 물리학이 여전히 활성화되어 있으면 작동 x
		// 따라서 무기를 장착하기 전에 장착된 무기 상태 설정을 호출하고 여기에서 물리 및 충돌 비활성화를 다시 처리해줘야 됨.
		break;
	}
}




void AWeapon::ShowPickupWidget(bool bShowWidget)
{
	if (PickupWidget)
	{
		PickupWidget->SetVisibility(bShowWidget);
	}
}

void AWeapon::Fire(const FVector& HitTarget)
{
	if (FireAnimation)
	{
		WeaponMesh->PlayAnimation(FireAnimation, false); // 반복 여부
	}
	if (CasingClass)
	{
		const USkeletalMeshSocket* AmmoEjectSocket = WeaponMesh->GetSocketByName(FName("AmmoEject"));
		if (AmmoEjectSocket)
		{
			FTransform SocketTransform = AmmoEjectSocket->GetSocketTransform(WeaponMesh);
			
			// 탄피는 선동자나 소유자 필요없음
			UWorld* World = GetWorld();
			if (World)
			{
				World->SpawnActor<ACasing>(
					CasingClass,
					SocketTransform.GetLocation(),
					SocketTransform.GetRotation().Rotator()
				);
			}
		}
	}
	SpendRound();
}

void AWeapon::Dropped()
{
	SetWeaponState(EWeaponState::EWS_Dropped);
	FDetachmentTransformRules DetachRules(EDetachmentRule::KeepWorld, true);
	WeaponMesh->DetachFromComponent(DetachRules);
	SetOwner(nullptr);
	BlasterOwnerCharacter = nullptr; // 이거 해야 무기에 남은 총알 수로 갱신됨.
	BlasterOwnerController = nullptr; // 
}

void AWeapon::AddAmmo(int32 AmmoToAdd)
{
	Ammo = FMath::Clamp(Ammo - AmmoToAdd, 0, MagCapacity);
	SetHUDAmmo();
}

bool AWeapon::IsEmpty()
{
	return Ammo <= 0;
}
