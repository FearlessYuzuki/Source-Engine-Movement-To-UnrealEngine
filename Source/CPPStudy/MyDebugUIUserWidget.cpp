// Fill out your copyright notice in the Description page of Project Settings.


#include "MyDebugUIUserWidget.h"
#include "Kismet/KismetSystemLibrary.h"
#include "PlayerCharacter.h"
#include "Components/Button.h"

void UMyDebugUIUserWidget::NativeConstruct()
{
	Super::NativeConstruct();
	
	
	if (FirstPersonButton)
	{
		FirstPersonButton->OnClicked.AddDynamic( this, &UMyDebugUIUserWidget::OnFirstPersonClicked );
	}
	if (ThirdPersonButton)
	{
		ThirdPersonButton->OnClicked.AddDynamic( this, &UMyDebugUIUserWidget::OnThirdPersonClicked );
	}
	if (KillGame)
	{
		KillGame->OnClicked.AddDynamic( this, &UMyDebugUIUserWidget::OnKillGameClicked );
	}
	if (ResumeGame)
	{
		ResumeGame->OnClicked.AddDynamic( this, &UMyDebugUIUserWidget::OnResumeCliceked);
	}
}


void UMyDebugUIUserWidget::OnFirstPersonClicked()
{
	APlayerCharacter* apc;
	apc = Cast<APlayerCharacter>(GetOwningPlayerPawn());
	
	APlayerController* roller = GetOwningPlayer();
	if (apc && IsValid(apc))
	{
		apc->SwitchToFirstCam();
		roller->SetShowMouseCursor(false);
		roller->SetInputMode(FInputModeGameOnly());
	}
	
	RemoveFromParent();
}

void UMyDebugUIUserWidget::OnThirdPersonClicked()
{
	APlayerCharacter* apc;
	apc = Cast<APlayerCharacter>(GetOwningPlayerPawn());
	
	APlayerController* roller = GetOwningPlayer();
	if (apc && IsValid(apc))
	{
		apc->SwitchToThirdCam();
		roller->SetShowMouseCursor(false);
		roller->SetInputMode(FInputModeGameOnly());
	}
	
	RemoveFromParent();
}

void UMyDebugUIUserWidget::OnKillGameClicked()
{
	UKismetSystemLibrary::QuitGame(this, nullptr, EQuitPreference::Quit,false);
}

void UMyDebugUIUserWidget::OnResumeCliceked()
{
	APlayerController* roller = GetOwningPlayer();
	if (roller && IsValid(roller))
	{
		roller->SetShowMouseCursor(false);
		roller->SetInputMode(FInputModeGameOnly());
	}
	
	RemoveFromParent();
}