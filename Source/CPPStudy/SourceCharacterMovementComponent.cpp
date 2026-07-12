// Fill out your copyright notice in the Description page of Project Settings.


#include "SourceCharacterMovementComponent.h"
#include "Engine/Engine.h"
#define SOURCEMAXAIRSPEED 2048

USourceCharacterMovementComponent::USourceCharacterMovementComponent()
{
	
}

void USourceCharacterMovementComponent::CalcVelocity(float DeltaTime, float Friction, bool bFluid,
	float BrakingDeceleration)
{
	//To switch air movement from UrealEngine to Source Style(Quake Style)
	if (IsFalling())
	{
		ApplySouceStyleAirMovement(DeltaTime);
	}
	else
	{
		Super::CalcVelocity(DeltaTime, Friction, bFluid, BrakingDeceleration);
	}
}

void USourceCharacterMovementComponent::ApplySouceStyleAirMovement(float DeltaTime)
{
	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 0, FColor::Blue,TEXT("Source air calc input"));
	}
	FVector wishdir = Acceleration.GetSafeNormal();
	float wishSpeed = Acceleration.Size();
	float acceleration = Sv_AirAcceleration;
	
	if (wishSpeed > GetCapppingAirAccleration())
	{
		wishSpeed = GetCapppingAirAccleration();
	}
	
	AirAcceleration(wishdir,wishSpeed,acceleration,DeltaTime);
}

void USourceCharacterMovementComponent::AirAcceleration(FVector wishdir, float wishSpeed, float acceleration,
                                                        float DeltaTime)
{
	float addSpeed = 0.0f;
	float currentspeed = 0.0f;
	float accelspeed = 0.0f;
	if (!IsFalling())
	{
		return;
	}
	
	currentspeed =Velocity.Dot(wishdir);
	
	addSpeed = wishSpeed - currentspeed;
	
	if (addSpeed <= 0) {return;};
	
	accelspeed = wishSpeed*DeltaTime*acceleration;
	
	if (accelspeed >addSpeed) {accelspeed = addSpeed;}
	
	Velocity += accelspeed*wishdir;
}

