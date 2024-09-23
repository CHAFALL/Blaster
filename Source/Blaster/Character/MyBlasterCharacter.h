// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "MyBlasterCharacter.generated.h"

UCLASS()
class BLASTER_API AMyBlasterCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	AMyBlasterCharacter();
	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

protected:
	virtual void BeginPlay() override;

	void MoveForward(float Value);
	void MoveRight(float Value);
	void Turn(float Value);
	void LookUp(float Value);

private:
	UPROPERTY(VisibleAnywhere, Category = Camera)
	class USpringArmComponent* CameraBoom;

	UPROPERTY(VisibleAnywhere, Category = Camera)
	class UCameraComponent* FollowCamera;

	// OverheadWidget 변수를 청사진에 노출하지만, 외부에서 직접 수정할 수 없도록 "BlueprintReadOnly"로 설정.
	// "AllowPrivateAccess = true"로 설정하여 클래스 내부에서만 접근 가능하게 함.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	class UWidgetComponent* OverheadWidget;

public:	
	
	
};

// 한글 주석 테스트