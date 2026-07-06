// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "EnhancedInputSubsystems.h"
#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "MyPawn.generated.h"


class UInputComponent;
class USkeletalMeshComponent;
class UCameraComponent;
class UInputAction;
struct FInputActionValue;

UCLASS()
class CPPSTUDY_API AMyPawn : public APawn
{
	GENERATED_BODY()

public:
	// Sets default values for this pawn's properties
	AMyPawn();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	

	//Moving//
	UPROPERTY(EditAnywhere,BlueprintReadOnly,Category = "Input",meta = (AllowPrivateAccess = true))
	UInputAction* MoveAction;
	
	void MoveInput(const FInputActionValue& Value);
	
protected:
		
	UPROPERTY(EditAnywhere,BlueprintReadOnly,Category="Input",meta = (AllowPrivateAccess = true))
	UInputMappingContext* Input;
	
	UFUNCTION(BlueprintCallable, Category="Input")
	virtual void DoMove(float Right, float Forward);
	
public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	
};
