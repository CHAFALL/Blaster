


#include "LagCompensationComponent.h"
#include "Blaster/Character/MyBlasterCharacter.h"
#include "Components/BoxComponent.h"
#include "DrawDebugHelpers.h"

ULagCompensationComponent::ULagCompensationComponent()
{
	PrimaryComponentTick.bCanEverTick = true;

}


void ULagCompensationComponent::BeginPlay()
{
	Super::BeginPlay();

	FFramePackage Package;
	SaveFramePackage(Package);
	ShowFramePackage(Package, FColor::Orange);
}

void ULagCompensationComponent::SaveFramePackage(FFramePackage& Package)
{
	// GetOwner - 해당 컴포넌트를 소유한.
	Character = Character == nullptr ? Cast<AMyBlasterCharacter>(GetOwner()) : Character;
	if (Character)
	{
		Package.Time = GetWorld()->GetTimeSeconds();
		for (auto& BoxPair : Character->HitCollisionBoxes)
		{
			FBoxInformation BoxInfomation;
			BoxInfomation.Location = BoxPair.Value->GetComponentLocation(); // 해당 상자 구성요소의 월드 공간 위치
			BoxInfomation.Rotation = BoxPair.Value->GetComponentRotation();
			BoxInfomation.BoxExtent = BoxPair.Value->GetScaledBoxExtent(); // 현재 값으로 크기가 조정된 상자 범위
			Package.HitBoxInfo.Add(BoxPair.Key, BoxInfomation);
		}
	}
}

void ULagCompensationComponent::ShowFramePackage(const FFramePackage& Package, const FColor& Color)
{
	for (auto& BoxInfo : Package.HitBoxInfo)
	{
		DrawDebugBox(
			GetWorld(),
			BoxInfo.Value.Location,
			BoxInfo.Value.BoxExtent,
			FQuat(BoxInfo.Value.Rotation),
			Color,
			true
		);
	}
}


void ULagCompensationComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

}


