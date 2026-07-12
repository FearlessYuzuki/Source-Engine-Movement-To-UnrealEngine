// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "SourceCharacterMovementComponent.generated.h"

/**
 *  Based On Source SDK - 2013
 */
UCLASS()
class CPPSTUDY_API USourceCharacterMovementComponent : public UCharacterMovementComponent
{
	GENERATED_BODY()

public:
	USourceCharacterMovementComponent();	

protected:
	virtual void CalcVelocity(float DeltaTime, float Friction, bool bFluid, float BrakingDeceleration) override;
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere,Category="Sv_AirAccleration")
	float Sv_AirAcceleration = 100.0f;
	
	virtual float GetCapppingAirAccleration(void) {return 30.0f;};
private:
	void ApplySouceStyleAirMovement(float DeltaTime);
	
	void AirAcceleration(FVector wishdir, float wishSpeed, float acceleration, float DeltaTime);
};
