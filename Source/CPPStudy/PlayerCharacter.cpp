// Fill out your copyright notice in the Description page of Project Settings.

#include "EnhancedInputComponent.h"
#include "GameFramework/Character.h"
#include "Components/InputComponent.h"
#include "InputActionValue.h"
#include "InputMappingContext.h"
#include "Engine/Engine.h"
#include "EnhancedInputSubsystems.h"
#include "PlayerCharacter.h"



// Sets default values
APlayerCharacter::APlayerCharacter()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void APlayerCharacter::BeginPlay()
{
	Super::BeginPlay();
	// 先拿到 PlayerController
	if (TObjectPtr<APlayerController> PC = Cast<APlayerController>(GetController()))
	{
		// 再拿 LocalPlayer
		if (TObjectPtr<ULocalPlayer> LP = PC->GetLocalPlayer())
		{
			// 再拿子系统
			if (UEnhancedInputLocalPlayerSubsystem* Sub =
				LP->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>())
			{
				// 注册你的 IMC
				Sub->AddMappingContext(IMC_Choice, 0);
			}
		}
	}
	//TODO:Hard to understand why official document edit one more cpp file to register the IMC event but i dont know how it works;
}

// Called every frame
void APlayerCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

// Called to bind functionality to input
void APlayerCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		//Look Trigger
		EnhancedInputComponent->BindAction(LookAction,ETriggerEvent::Triggered,this,&APlayerCharacter::MouseLookInput);
		
		//Move Trigger
		EnhancedInputComponent->BindAction(MoveAction,ETriggerEvent::Triggered,this,&APlayerCharacter::MoveInput);	
		
		//Jump Trigger
		EnhancedInputComponent->BindAction(JumpAction,ETriggerEvent::Started,this,&APlayerCharacter::DoJumpStart);
		EnhancedInputComponent->BindAction(JumpAction,ETriggerEvent::Completed,this,&APlayerCharacter::DoJumpEnd);
	}
}

void APlayerCharacter::MoveInput(const FInputActionValue& Value)
{
	GEngine->AddOnScreenDebugMessage(-1 , 0.0f, FColor::Green,"Input Detctive");
	
	FVector2D Vector2D = Value.Get<FVector2D>();
	
	Domove(Vector2D.X,Vector2D.Y);
}

void APlayerCharacter::MouseLookInput(const FInputActionValue& Value)
{
	GEngine->AddOnScreenDebugMessage(-1 , 0.0f, FColor::Green,"Mouse Spining Detective");
	FVector2D V = Value.Get<FVector2D>();
	
	DoLook(V.X,V.Y);
}

void APlayerCharacter::Domove(float right, float left)
{
	if (GetController())
	{
		AddMovementInput(GetActorRightVector(), right);
		AddMovementInput(GetActorForwardVector(), left);
	}
}

void APlayerCharacter::DoLook(float Yaw, float Pitch)
{
	if (GetController())
	{
		AddControllerYawInput(Yaw);
		AddControllerPitchInput(Pitch);
	}
	
}

void APlayerCharacter::DoJumpStart()
{
	Jump();
}

void APlayerCharacter::DoJumpEnd()
{
	StopJumping();
}

