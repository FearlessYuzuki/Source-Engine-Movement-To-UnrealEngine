// Fill out your copyright notice in the Description page of Project Settings.

#include "SourceCharacterMovementComponent.h"
#include "GameFramework/Character.h"
#include "PlayerCharacter.h"
#include "Engine/Engine.h"

#define SOURCEMAXAIRSPEED 1000
#define DEFAULTSPEED 635

USourceCharacterMovementComponent::USourceCharacterMovementComponent()
{
	
}

void USourceCharacterMovementComponent::CalcVelocity(float DeltaTime, float Friction, bool bFluid,
	float BrakingDeceleration)
{
	/* -------------------------------
			 * Debug Area (DevMode only)
	 ---------------------------------*/
	if (const APlayerCharacter* PlayerChar = Cast<APlayerCharacter>(CharacterOwner))
	{
		if (PlayerChar->bDevMode)
		{
			if (GEngine)
			{
				GEngine->AddOnScreenDebugMessage(-1, 0, FColor::Red,TEXT("Custom CalcVelocity Avaliable"));
			}
			DrawDebugLine(GetWorld(),GetActorLocation(),GetActorLocation()+Acceleration.GetSafeNormal2D(),FColor::Green,false,-1,0,3.0f);
			DrawDebugLine(GetWorld(),GetActorLocation(),GetActorLocation() + Velocity.GetSafeNormal2D() * 300.0f,FColor::Red,false,-1.0f,0,3.0f);

			FVector InputAcceleration = Acceleration;

			GEngine->AddOnScreenDebugMessage(-1,0.0f,FColor::Yellow,FString::Printf(TEXT("Acceleration: X=%f Y=%f Z=%f Size2D=%f")
				,InputAcceleration.X,InputAcceleration.Y,InputAcceleration.Z,InputAcceleration.Size2D()));
		}
	}
	/* -------------------------------
			 * Debug Area
	---------------------------------*/
	
	//声明 我实在是不想写英文了 因为我本来觉得英文很简单 (虽然确实是这样) 但是这里的SDK逻辑我实在是不想用英语理解
	//首先源代码有wishspeed wishspd spd wishdir 等一系列变量 但是注意这里的某一个变量其实是用来暂存数据的 也就是Valve的引擎开发者在设计这一部分的时候
	//拿了一个变量用来寄存当前角色的移动的速度的速率 是一个标量 但是速度是矢量 是有速度的 而这里normalize的向量是又把方向和长度一起normalize的 也就是这里 Normlize一个向量其实是既Normalize了向量 但是同时把向量的值返回了
	//所以这里很麻烦的点就是名称问题以及注释问题
	
	/*-------------------------------------------------------------
			this part of code is refer to Source SDK 2013
	---------------------------------------------------------------*/
	
	// ================= Settings =================
	Sv_AirAcceleration = 12;
	GroundFriction = 8.f;
	MaxAcceleration = 600.f;
	if (!HasValidData() || HasAnimRootMotion() || DeltaTime < MIN_TICK_TIME || (CharacterOwner && CharacterOwner->GetLocalRole() == ROLE_SimulatedProxy && !bWasSimulatingRootMotion))
	{
		return;
	}
	
	//To switch air movement from UrealEngine to Source Style(Quake Style)
	if (IsFalling())
	{
		AirMove(DeltaTime);
		return;
	}
	
	if (IsMovingOnGround())
	{
		ApplyFriction(DeltaTime);	
		WalkMove(DeltaTime);
		return;
	}
	
	
}

void USourceCharacterMovementComponent::AirMove(float DeltaTime)
{
	/*if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 0, FColor::Blue,TEXT("Source air calc input"));
	}*/
	FVector wishdir = Acceleration.GetSafeNormal2D();
	float wishSpeed = Acceleration.Size2D();
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
	
	if (addSpeed <= 0)
	{
		return;
	}
	
	accelspeed = wishSpeed*DeltaTime*acceleration;
	
	if (accelspeed >addSpeed)
	{
		accelspeed = addSpeed;
	}
	
	Velocity += accelspeed*wishdir;
	
}

void USourceCharacterMovementComponent::WalkMove(float DeltaTime)
{
	FVector wishdir = Acceleration.GetSafeNormal2D();
	float wishSpeed = Acceleration.Size2D();
	float acceleration = Acceleration.Size2D();
	bool bCanAccele;
	
	if (Velocity.Z != 0)
	{
		Velocity.Z = 0;
	}
	
	/*if (Velocity.Size()<MaxGroundSpeed && Acceleration.Size()!=0)
	{
		bCanAccele = true;
	}
	else
	{
		bCanAccele = false;
	}*/
	bCanAccele = true;
	
	//TODO:Chara move like slide and if chara sped = 0 cant accel
	//TODO: Figure out what the hell UE controls Chara move and Figure Source Engine
	
	if (bCanAccele)
	{
		GroundAccelerate(wishdir,wishSpeed,acceleration,DeltaTime);
	}
	
	if (!bCanAccele)
	{
		return;
	}
	
}

void USourceCharacterMovementComponent::GroundAccelerate(FVector wishdir, float wishSpeed, float acceleration,
														 float DeltaTime)
{
	float addspeed;
	float accelspeed;
	float crtspeed;
	float Friction = 1/GroundFriction;
	
	crtspeed = Velocity.Dot(wishdir);
	addspeed = wishSpeed - crtspeed;
	if (addspeed <= 0)
	{
		return;
	}
	
	accelspeed = wishSpeed*DeltaTime*acceleration*Friction;//Firction need RayTracing
	
	if (accelspeed>addspeed)
	{
		accelspeed = addspeed;
	}
	
	Velocity += accelspeed*wishdir;
}

void USourceCharacterMovementComponent::ApplyFriction(float DeltaTime)
{
	float Speeding = Velocity.Size2D();
	float Control,NewSpeed,drop,Friction;
	if (Speeding < 1.5f)
	{
		Velocity.X = 0;
		Velocity.Y = 0;
		return;
	}
	
	drop = 0;
	Control = (Speeding < Sv_StopSpeed)?Sv_StopSpeed:Speeding; 
	Friction = FMath::Max(GetGroundFriction(DeltaTime),0.0f);
	drop += Control*Friction*DeltaTime;
	NewSpeed = Speeding - drop;
	
	if (NewSpeed <0.f)
	{
		NewSpeed = 0;
	}
	
	NewSpeed /= Speeding;
	
	Velocity = VectorScale(Velocity,NewSpeed);
	
}	



//----------Tools Area ---------//
FVector USourceCharacterMovementComponent::VectorScale(const FVector& InVector, double scale)
{
	return InVector * scale;
}

float USourceCharacterMovementComponent::GetGroundFriction(float DeltaTime)
{
	return GroundFriction;
}

