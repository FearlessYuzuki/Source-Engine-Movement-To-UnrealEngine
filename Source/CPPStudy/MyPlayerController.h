// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "SourceCharacterMovementComponent.h"
#include "GameFramework/PlayerController.h"
#include "MyPlayerController.generated.h"
/**
 * 
 */

class UInputMappingContext;

UCLASS(abstract, config = "Game")
class CPPSTUDY_API AMyPlayerController : public APlayerController
{
	GENERATED_BODY()
public:
	AMyPlayerController();
	
protected:
	
	UPROPERTY(EditAnywhere,category="Input|Input Mappings")
	TArray<TObjectPtr<UInputMappingContext>> InputMappingContexts;
	
	virtual void BeginPlay() override;
	
	virtual void SetupInputComponent() override;
	
	
};
