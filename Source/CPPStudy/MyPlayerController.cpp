// Fill out your copyright notice in the Description page of Project Settings.


#include "MyPlayerController.h"
#include "EnhancedInputSubsystems.h"
#include "InputMappingContext.h"
#include "Engine/LocalPlayer.h"

AMyPlayerController::AMyPlayerController()
{
	
}

void AMyPlayerController::BeginPlay()
{
	Super::BeginPlay();
	
	
}

void AMyPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();
	
	if (IsLocalPlayerController())
	{
		if (UEnhancedInputLocalPlayerSubsystem * Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
		{
			for (UInputMappingContext* CurrentContext : InputMappingContexts)
			{
				Subsystem->AddMappingContext(CurrentContext,0);
			}
		}
	}
}
