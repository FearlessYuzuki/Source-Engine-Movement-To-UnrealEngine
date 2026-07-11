// Fill out your copyright notice in the Description page of Project Settings.


#include "SourceCharacterMovementComponent.h"
#include "Engine/Engine.h"


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
	
}

void USourceCharacterMovementComponent::AirAcceleration(FVector wishdir, float wishSpeed, float acceleration,
                                                        float DeltaTime)
{
	
}

