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

	// OverheadWidget ������ û������ ����������, �ܺο��� ���� ������ �� ������ "BlueprintReadOnly"�� ����.
	// "AllowPrivateAccess = true"�� �����Ͽ� Ŭ���� ���ο����� ���� �����ϰ� ��.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	class UWidgetComponent* OverheadWidget;

public:	
	

};
