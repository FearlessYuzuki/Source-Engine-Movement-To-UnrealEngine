// Fill out your copyright notice in the Description page of Project Settings.


#include "SourceCharacterMovementComponent.h"
#include "Engine/Engine.h"
#define SOURCEMAXAIRSPEED 2048
#define DEFAULTSPEED 635

//TODO:Source Ground Speed and UE friction override

USourceCharacterMovementComponent::USourceCharacterMovementComponent()
{
	
}

void USourceCharacterMovementComponent::CalcVelocity(float DeltaTime, float Friction, bool bFluid,
	float BrakingDeceleration)
{
	//声明 我实在是不想写英文了 因为我本来觉得英文很简单 (虽然确实是这样) 但是这里的SDK逻辑我实在是不想用英语理解 解放一点大脑的思考能力
	//首先源代码有wishspeed wishspd spd wishdir 等一系列变量 但是注意这里的某一个变量其实是用来暂存数据的 也就是Valve的引擎开发者在设计这一部分的时候
	//拿了一个变量用来寄存当前角色的移动的速度的速率 是一个标量 但是速度是矢量 是有速度的 而这里normalize的向量是又把方向和长度一起normalize的
	//所以这里很麻烦的点就是名称问题以及注释问题
	
	/*-------------------------------------------------------------
			this part of code is refer to Source SDK 2013
	---------------------------------------------------------------*/
	
	if (HasValidData() || HasAnimRootMotion() || DeltaTime < MIN_TICK_TIME)
	{
		return;
	}
	
	FVector wishdirection=Velocity.GetSafeNormal2D();
	FVector wishVelocity;
	double CurrentSpeed = Velocity.Size2D();
	double wishSpeed;
	
	if (CurrentSpeed <= 0)
	{
		return;
	}
	
	//To switch air movement from UrealEngine to Source Style(Quake Style)
	if (IsFalling())
	{
		ApplySouceStyleAirMovement(DeltaTime);
		return;
	}
	else //Ground MoveSpeed Calc
	{
		
		//Super::CalcVelocity(DeltaTime, Friction, bFluid, BrakingDeceleration);
		if (CurrentSpeed==0 || !HasValidData() || HasAnimRootMotion() || DeltaTime < MIN_TICK_TIME)
		{
			return;
		}
		
		//GetLocalPlayerSpeed to limit or calc speed(dont needed)
		//current idea is limit max speed or use calculation (such as Friction) to reduce speed too high
		//both idea can be use because thats Origin Source Code does in Source SDK 2013
		
		Velocity.Z = 0; //Remove Z axis Velocity
		
		//UE5.6Upper Recommend to use Velocity to time X Y axis Vector to use Graphic Booster to reduce the time in calc
		wishSpeed=wishVelocity.Size2D();
		
		if (Velocity.Size2D() < SOURCEMAXAIRSPEED && Velocity.Size2D() !=0)
		{
			//here to calc speed or cap speed
		}
		
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

