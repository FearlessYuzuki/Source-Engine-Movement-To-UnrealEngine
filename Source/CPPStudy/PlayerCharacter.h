// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "EnhancedInputComponent.h"
#include "GameFramework/Character.h"
#include "InputMappingContext.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "PlayerCharacter.generated.h"

class UInputComponent;
class USkeletalMeshComponent;
class UCameraComponent;
class UInputAction;
struct FInputActionValue;

UCLASS()
class CPPSTUDY_API APlayerCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	APlayerCharacter();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	void MoveInput(const FInputActionValue &Value);
	void MouseLookInput(const FInputActionValue &Value);
	
protected:
	UPROPERTY(EditAnywhere,BlueprintReadOnly,Category = "Input",meta = (AllowPrivateAccess = true))
	UInputAction * MoveAction;
	
	UPROPERTY(EditAnywhere,BlueprintReadOnly,Category="Input",meta = (AllowPrivateAccess = true))
	UInputMappingContext* IMC_Choice;
	
	UPROPERTY(EditAnywhere,BlueprintReadOnly,Category = "Input",meta = (AllowPrivateAccess = true))
	UInputAction * LookAction;
	
	UFUNCTION(BlueprintCallable, Category="Input")
	virtual void Domove (float Right, float Left);
	UFUNCTION(BlueprintCallable, Category="Input")
	virtual void DoLook (float Yaw, float Pitch);
	
public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

};
