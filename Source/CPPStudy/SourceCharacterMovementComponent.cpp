// Fill out your copyright notice in the Description page of Project Settings.

#include "SourceCharacterMovementComponent.h"
#include "GameFramework/Character.h"
#include "Camera/CameraComponent.h"
#include "PlayerCharacter.h"
#include "Engine/Engine.h"
#include "Math/RotationMatrix.h"



USourceCharacterMovementComponent::USourceCharacterMovementComponent()
{
	//H3 fix: one-time init moved out of CalcVelocity. Runtime values are owned by UPROPERTYs.
	GroundFriction = 8.f;
	MaxAcceleration = 600.f;
}

void USourceCharacterMovementComponent::SetMovementInput(float ForwardIn, float SideIn)
{
	//RouteB fix: input layer must write 0 on release, or the scalars stick
	mv_forwardMove = FMath::Clamp(ForwardIn, -1.0f, 1.0f);
	mv_sideMove = FMath::Clamp(SideIn, -1.0f, 1.0f);
}

//RouteB fix: view yaw basis = Source AngleVectors + Z clear (gm.cpp:1760); ControlRotation, not the camera
void USourceCharacterMovementComponent::GetViewBasisVectors(FVector& OutForward, FVector& OutRight) const
{
	const float ViewYaw = CharacterOwner ? CharacterOwner->GetControlRotation().Yaw : 0.0f;
	const FRotator YawRotation(0.0f, ViewYaw, 0.0f);
	const FMatrix ViewBasis = FRotationMatrix(YawRotation);

	OutForward = ViewBasis.GetUnitAxis(EAxis::X);
	OutRight   = ViewBasis.GetUnitAxis(EAxis::Y);
}

FVector USourceCharacterMovementComponent::CalcWishVel() const
{
	float F = mv_forwardMove;
	float S = mv_sideMove;
	const float InputSpdSq = F * F + S * S;
	if (InputSpdSq > 1.0f) //RouteB fix: diagonal clamp, W+D must not give sqrt(2) wishspeed
	{
		const float Ratio = 1.0f / FMath::Sqrt(InputSpdSq);
		F *= Ratio;
		S *= Ratio;
	}

	FVector ViewForward, ViewRight;
	GetViewBasisVectors(ViewForward, ViewRight);
	FVector WishVel = ViewForward * F + ViewRight * S;
	WishVel.Z = 0.0f;

	return WishVel;
}

//RouteB fix: MaxGroundSpeed plays mv->m_flMaxSpeed for both WalkMove and AirMove
void USourceCharacterMovementComponent::CalcWishDirAndSpeed(FVector& OutWishDir, float& OutWishSpeed) const
{
	const FVector WishVel = CalcWishVel();
	const float WishSpeedScalar = WishVel.Size();

	OutWishDir = WishVel.GetSafeNormal();
	OutWishSpeed = WishSpeedScalar * MaxGroundSpeed;

	if (OutWishSpeed > MaxGroundSpeed)
	{
		OutWishSpeed = MaxGroundSpeed;
	}
}

void USourceCharacterMovementComponent::AirMove(float DeltaTime)
{
	FVector wishdir = FVector::ZeroVector;
	float wishSpeed = 0.0f;
	CalcWishDirAndSpeed(wishdir, wishSpeed);

	AirAcceleration(wishdir, wishSpeed, Sv_AirAcceleration, DeltaTime);
}

void USourceCharacterMovementComponent::AirAcceleration(FVector wishdir, float wishSpeed, float acceleration,
                                                        float DeltaTime)
{
	if (!IsFalling())
	{
		return;
	}

	if (wishSpeed <= 0.0f || wishdir.IsNearlyZero())
	{
		return;
	}

	const float WishSpd = FMath::Min(wishSpeed, Sv_AirCappingSpeed); //RouteB fix: cap limits addspeed only

	float addSpeed = 0.0f;//init
	const float crtspeed = Velocity.Dot(wishdir);
	float accelspeed = 0.0f;//init

	addSpeed = WishSpd - crtspeed;

	if (addSpeed <= 0)
	{
		return;
	}

	accelspeed = wishSpeed * DeltaTime * acceleration; //RouteB fix: unclamped wishSpeed here (gm.cpp:1734)

	if (accelspeed > addSpeed)
	{
		accelspeed = addSpeed;
	}

	Velocity += accelspeed * wishdir;
}

void USourceCharacterMovementComponent::WalkMove(float DeltaTime)
{
	FVector wishdir = FVector::ZeroVector;
	float wishSpeed = 0.0f;
	CalcWishDirAndSpeed(wishdir, wishSpeed);

	if (Velocity.Z != 0)
	{
		Velocity.Z = 0;
	}

	//TODO:Chara move like slide and if chara sped = 0 cant accel
	//TODO: Figure out what the hell UE controls Chara move and Figure Source Engine

	Accelerate(wishdir, wishSpeed, Sv_Accelerate, DeltaTime);
}

void USourceCharacterMovementComponent::Accelerate(FVector wishdir, float wishSpeed, float acceleration,
														 float DeltaTime)
{
	float addspeed;
	float accelspeed;
	float crtspeed;
	
	crtspeed = Velocity.Dot(wishdir);
	addspeed = wishSpeed - crtspeed;
	if (addspeed <= 0)
	{
		return;
	}
	
	accelspeed = wishSpeed*DeltaTime*acceleration;//P3 fix: Source is accelspeed = accel * wishspeed * dt, no friction multiplier
	
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
	if (Speeding < 0.1f) //P4 fix: Source snap threshold
	{
		Velocity.X = 0;
		Velocity.Y = 0;
		return;
	}
	
	drop = 0;
	Control = (Speeding < Sv_StopSpeed)?Sv_StopSpeed:Speeding; 
	Friction = FMath::Max(Sv_Friction,0.0f); //P4 fix: read sv_friction UPROPERTY
	drop += Control*Friction*DeltaTime;
	NewSpeed = Speeding - drop;
	
	if (NewSpeed <0.f)
	{
		NewSpeed = 0;
	}
	
	NewSpeed /= Speeding;
	
	Velocity = VectorScale(Velocity,NewSpeed);
	
}	


void USourceCharacterMovementComponent::CalcVelocity(float DeltaTime, float Friction, bool bFluid,
	float BrakingDeceleration)
{
	/* -------------------------------
			 * Debug Area (DevMode only)
	 ---------------------------------*/
	//RouteB fix: Acceleration is always 0 now, read the scalar channel instead
	if (const APlayerCharacter* PlayerChar = Cast<APlayerCharacter>(CharacterOwner))
	{
		if (PlayerChar->bDevMode)
		{
			if (GEngine)
			{
				GEngine->AddOnScreenDebugMessage(-1, 0, FColor::Red,TEXT("Custom CalcVelocity Avaliable"));
			}

			FVector WishDir = FVector::ZeroVector;
			float WishSpeed = 0.0f;
			CalcWishDirAndSpeed(WishDir, WishSpeed);

			DrawDebugLine(GetWorld(),GetActorLocation(),GetActorLocation() + CalcWishVel() * DebugLineLength,FColor::Yellow,false,-1.0f,0,3.0f);
			DrawDebugLine(GetWorld(),GetActorLocation(),GetActorLocation() + Velocity.GetSafeNormal2D() * 300.0f,FColor::Red,false,-1.0f,0,3.0f);

			GEngine->AddOnScreenDebugMessage(-1,0.0f,FColor::Yellow,FString::Printf(
				TEXT("mv: f=%.3f s=%.3f | wishdir: X=%.3f Y=%.3f | wishspeed=%.1f | vel=%.1f"),
				mv_forwardMove, mv_sideMove, WishDir.X, WishDir.Y, WishSpeed, Velocity.Size2D()));
		}
	}
	
	if (APlayerCharacter* RenderCamPointer = Cast<APlayerCharacter>(CharacterOwner))
	{
		if (RenderCamPointer)
		{
			DrawCameraDebugline(RenderCamPointer->bDevMode);
		}
		else
		{
			return;
		}
	}
	
	/* -------------------------------
			 * Debug Area
	---------------------------------*/
	
	/*-------------------------------------------------------------
			this part of code is refer to Source SDK 2013
	---------------------------------------------------------------*/
	
	// ================= Settings =================
	//H3 fix: per-tick hardcode removed. Init lives in the constructor; Details panel values now take effect live.
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



//----------Tools Area ---------//
FVector USourceCharacterMovementComponent::VectorScale(const FVector& InVector, double scale)
{
	return InVector * scale;
}

float USourceCharacterMovementComponent::GetGroundFriction(float DeltaTime)
{
	return GroundFriction;
}

void USourceCharacterMovementComponent::CalcVector(const FRotator &Angles, float Forward)
{
	
}

void USourceCharacterMovementComponent::DrawCameraDebugline(bool bDevMode)
{
	APlayerCharacter* PlayerChar = Cast<APlayerCharacter>(CharacterOwner);
	if (!PlayerChar || !bDevMode)
	{
		return;
	}

	FVector ViewForward, ViewRight;
	GetViewBasisVectors(ViewForward, ViewRight);
	const FVector Origin = GetActorLocation();

	DrawDebugLine(GetWorld(), Origin, Origin + ViewForward * DebugLineLength, FColor::Green,false,-1.0f,0,3.0f);
	DrawDebugLine(GetWorld(), Origin, Origin + ViewRight   * DebugLineLength, FColor::Blue, false,-1.0f,0,3.0f);
	DrawDebugLine(GetWorld(), Origin, Origin + CalcWishVel() * DebugLineLength, FColor::Yellow,false,-1.0f,0,3.0f);
}
