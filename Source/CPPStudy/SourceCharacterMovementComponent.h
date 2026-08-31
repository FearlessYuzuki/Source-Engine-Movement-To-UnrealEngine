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
	virtual void CalcVelocity(float DeltaTime, float Friction, bool bFluid, float BrakingDeceleration) override;
	
protected:
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere,Category="Sv_AirAccleration")
	float Sv_AirAcceleration = 100.0f;
	UPROPERTY(BlueprintReadWrite, EditAnywhere,Category="Sv_AirAccleration")
	float Sv_StopSpeed = 1.0f;
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere,Category="Basic Game settings")
	float MaxGroundSpeed = 600.f;
	
	virtual float GetCapppingAirAccleration(void) {return 100.0f;}
	
	void AirMove(float DeltaTime);
	
	void AirAcceleration(FVector wishdir, float wishSpeed, float acceleration, float DeltaTime);
	
	void WalkMove(float DeltaTime);
	
	void ApplyFriction(float DeltaTime);
	
	void GroundAccelerate(FVector wishdir, float wishSpeed, float acceleration, float DeltaTime);
	
	FVector VectorScale(const FVector &InVector, double scale);
	
	float GetGroundFriction(float DeltaTime);
};
