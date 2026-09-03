// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "InputMappingContext.h"
#include "Camera/CameraComponent.h"
#include "MyDebugUIUserWidget.h"
#include "GameFramework/SpringArmComponent.h"
#include "PlayerCharacter.generated.h"


class UInputComponent;
class USkeletalMeshComponent;
class UCameraComponent;
class UInputAction;
class UUserWidget;
struct FInputActionValue;

UCLASS()
class CPPSTUDY_API APlayerCharacter : public ACharacter
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere,BlueprintReadOnly,Category="Components",meta = (AllowPrivateAccess = true))
	TObjectPtr<UCameraComponent> FirstPersonCamera;
	
	UPROPERTY(EditAnywhere,BlueprintReadOnly,Category="Components",meta = (AllowPrivateAccess = true))
	TObjectPtr<USpringArmComponent> FirstPersonSpringArm;

	UPROPERTY(EditAnywhere,BlueprintReadOnly,Category="Components",meta = (AllowPrivateAccess = true))
	TObjectPtr<UCameraComponent> DebugCamera;
	
	UPROPERTY(EditAnywhere,BlueprintReadOnly,Category="Components",meta = (AllowPrivateAccess = true))
	TObjectPtr<USpringArmComponent> DebugSpringArm;

	
public:
	
	virtual void SwitchToFirstCam();
	virtual void SwitchToThirdCam();
	
public:
	virtual void BeginPlay() override;
	
	//
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Tools|DevMode")
	bool bDevMode = false;
	
	// Sets default values for this character's properties
	APlayerCharacter(const FObjectInitializer& ObjectInitializer);
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Tools|AutoBhoping")
	bool AutoBhopFunction;
	
	//FunctionalTools;
	//Blueprintcalledable UserTools
	UPROPERTY(EditAnywhere,BlueprintReadOnly,Category="Tools")
	bool SpeedShowingSwitch;
	
public:
	UPROPERTY(BlueprintReadWrite,EditAnywhere,Category="DebugUI")
	TSubclassOf<class UUserWidget> DebugUI;
	
	UPROPERTY()
	TObjectPtr<UUserWidget> DebugWidgetInstance;
	
protected:
	// Called when the game starts or when spawned
	void MoveInput(const FInputActionValue &Value);
	void MouseLookInput(const FInputActionValue &Value);
	void DebugMenuCalled(const FInputActionValue &Value);
	
protected:
	//Action UPRO
	UPROPERTY(EditAnywhere,BlueprintReadOnly,Category = "Input",meta = (AllowPrivateAccess = true))
	TObjectPtr<UInputAction> MoveAction;
	
	UPROPERTY(EditAnywhere,BlueprintReadOnly,Category = "Input",meta = (AllowPrivateAccess = true))
	TObjectPtr<UInputAction> JumpAction;
	
	UPROPERTY(EditAnywhere,BlueprintReadOnly,Category="Input",meta = (AllowPrivateAccess = true))
	TObjectPtr<UInputMappingContext> IMC_Choice;
	
	UPROPERTY(EditAnywhere,BlueprintReadOnly,Category = "Input",meta = (AllowPrivateAccess = true))
	TObjectPtr<UInputAction> LookAction;
	
	UPROPERTY(EditAnywhere,BlueprintReadOnly,Category = "Input",meta = (AllowPrivateAccess = true))
	TObjectPtr<UInputAction> DebugAction;
	
	//Action Ufunc	
	UFUNCTION(BlueprintCallable, Category="Input")
	virtual void Domove (float Right, float Left);
	UFUNCTION(BlueprintCallable, Category="Input")
	virtual void DoLook (float Yaw, float Pitch);
	UFUNCTION(BlueprintCallable, Category="Input")
	virtual void DoJumpStart();
	UFUNCTION(BlueprintCallable, Category="Input")
	virtual void DoJumpEnd();
	

	//FuncSelfEditTools
	virtual void ShowVelocity();

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	
	
};
