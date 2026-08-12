// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "SourceCharacterMovementComponent.generated.h"
/**
 *  Idea come from Source SDK - 2013
 */
UCLASS()
class CPPSTUDY_API USourceCharacterMovementComponent : public UCharacterMovementComponent
{
	GENERATED_BODY()

public:
	USourceCharacterMovementComponent();	

	UPROPERTY(EditAnywhere, Category = "Tools|AutoBhoping")
	bool EnableBunnyhoping;

	
protected:
	virtual void CalcVelocity(float DeltaTime, float Friction, bool bFluid, float BrakingDeceleration) override;
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere,Category="Sv_AirAccleration")
	float Sv_AirAcceleration = 100.0f;
	
	virtual float GetCapppingAirAccleration(void) {return 100.0f;}
	
	void AirMove(float DeltaTime);
	
	void AirAcceleration(FVector wishdir, float wishSpeed, float acceleration, float DeltaTime);
	
	void WalkMove(float DeltaTime);
	
	void GroundAccelerate(FVector wishdir, float wishSpeed, float acceleration, float DeltaTime);
	
	FVector VectorScale(const FVector &InVector, double scale);
};
