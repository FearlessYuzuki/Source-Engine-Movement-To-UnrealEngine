// Fill out your copyright notice in the Description page of Project Settings.


#include "SourceCharacterMovementComponent.h"
#include "Engine/Engine.h"
#include "PlayerCharacter.h"
#define SOURCEMAXAIRSPEED 2048

//TODO:Source Ground Speed and UE friction override

USourceCharacterMovementComponent::USourceCharacterMovementComponent()
{
	
}

void USourceCharacterMovementComponent::CalcVelocity(float DeltaTime, float Friction, bool bFluid,
	float BrakingDeceleration)
{
	/*--------------------
					this part of code is refer to Source SDK 2013
	--------------------*/
	
	
	FVector wishVelocity;
	
	double PlayerVelocity2D = Velocity.Size2D();
	
	//To switch air movement from UrealEngine to Source Style(Quake Style)
	if (IsFalling())
	{
		ApplySouceStyleAirMovement(DeltaTime);
	}
	else //Ground MoveSpeed Calc
	{
		
		//Super::CalcVelocity(DeltaTime, Friction, bFluid, BrakingDeceleration);
		if (PlayerVelocity2D==0 || !HasValidData() || HasAnimRootMotion() || DeltaTime < MIN_TICK_TIME)
		{
			return;
		}
		
		if (PawnOwner && PawnOwner->IsLocallyControlled())
		{
			//get local player (temp idea not use it for now)
		}
		// TODO: idea
		//GetLocalPlayerSpeed to limit or calc speed
		//current idea is limit max speed or use calculation (such as Friction) to reduce speed too high
		//both idea can be use because thats Origin Source Code does in Source SDK 2013
		
		Velocity.Z = 0; //Remove Z axis Velocity
		double VxNormal = VectorNormalize(Velocity.X);
		double VyNormal = VectorNormalize(Velocity.Y);
		//Velocity Normalization
		
		//Keep Z axis speed equals zero
		wishVelocity.Z = 0;
		
		//UE5.6Upper Recommend to use Velocity to time X Y axis Vector to use Graphic Booster to reduce the time in calc
		wishVelocity=Velocity*(VxNormal,VyNormal);
		
		//Copy X Y Velocity to Velocity
		Velocity.X = wishVelocity[0]*DeltaTime;
		Velocity.Y = wishVelocity[1]*DeltaTime;
		
		
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
	float addSpeed = 0.0f;//init
	float crtspeed = Velocity.Dot(wishdir);
	float accelspeed = 0.0f;//init
	if (!IsFalling())
	{
		return;
	}
	
	addSpeed = wishSpeed - crtspeed;
	
	if (addSpeed <= 0) {return;}
	
	accelspeed = wishSpeed*DeltaTime*acceleration;
	
	if (accelspeed >addSpeed) {accelspeed = addSpeed;}
	
	Velocity += accelspeed*wishdir;
}

