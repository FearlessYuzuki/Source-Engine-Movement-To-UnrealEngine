// Fill out your copyright notice in the Description page of Project Settings.

#include "MyPawn.h"
#include "EnhancedInputSubsystems.h"
#include "EnhancedInputComponent.h"
#include "InputMappingContext.h"    
#include "GameFramework/CharacterMovementComponent.h"// 必须引入，才能识别 IMC 类型
#include "GameFramework/PlayerController.h" // 必须引入，才能使用 APlayerController 和 GetLocalPlayer()
#include "Engine/Engine.h"
#include "InputAction.h"

// Sets default values
AMyPawn::AMyPawn()
{
 	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void AMyPawn::BeginPlay()
{
	Super::BeginPlay();
}

// Called every frame
void AMyPawn::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

// Called to bind functionality to input
void AMyPawn::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	
	//GetIMC(Controller)
	if (APlayerController* PC = Cast<APlayerController>(GetController()))
	{
		if (ULocalPlayer* LocalPlayer = PC->GetLocalPlayer())
		{
			if (UEnhancedInputLocalPlayerSubsystem* LocalPlayerSubsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(LocalPlayer))
			{
				if (Input)
				{
					LocalPlayerSubsystem->AddMappingContext(Input, 0);
				}

			}
		}
	}
	  
	
	
	
	//BindAction
	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AMyPawn::MoveInput);
	}
}

void AMyPawn::MoveInput(const FInputActionValue& Value)
{
	GEngine->AddOnScreenDebugMessage(-1, 0.0f, FColor::Green, TEXT("Input Pressed"));
	FVector2D Movement2D = Value.Get<FVector2D>();
	DoMove(Movement2D.X, Movement2D.Y);
}


void AMyPawn::DoMove(float Right, float Forward)
{
	if (GetController())
	{
		AddMovementInput(GetActorRightVector(), Right);
		AddMovementInput(GetActorForwardVector(), Forward);
	}
}


