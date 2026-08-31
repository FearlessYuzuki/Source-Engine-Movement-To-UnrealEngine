// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "MyDebugUIUserWidget.generated.h"

class APlayerCharacter;
class UButton;
class UCameraComponent;
/**
 * 
 */
UCLASS()
class CPPSTUDY_API UMyDebugUIUserWidget : public UUserWidget
{
	GENERATED_BODY()


public:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> FirstPersonButton;
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> ThirdPersonButton;
protected:
	virtual void NativeConstruct() override;
	
	UFUNCTION()
	void OnFirstPersonClicked();

	UFUNCTION()
	void OnThirdPersonClicked();
};
