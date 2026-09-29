// Fill out your copyright notice in the Description page of Project Settings.

#include "PlayerCharacter.h"
#include "EnhancedInputComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/InputComponent.h"
#include "InputActionValue.h"
#include "Engine/Engine.h"
#include "SourceCharacterMovementComponent.h"
#include "GameFramework/PlayerStart.h"
#include "Kismet/GameplayStatics.h"

// Sets default values
APlayerCharacter::APlayerCharacter(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer.SetDefaultSubobjectClass<USourceCharacterMovementComponent>(ACharacter::CharacterMovementComponentName))
{
	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	
	//First Person Camera Component
	FirstPersonCamera = CreateDefaultSubobject<UCameraComponent>(FName("FirstPersonCamera"));
	FirstPersonSpringArm = CreateDefaultSubobject<USpringArmComponent>(FName("FirstPersonSpringArm"));
	
	FirstPersonSpringArm->SetupAttachment(RootComponent);
	FirstPersonCamera->SetupAttachment(FirstPersonSpringArm, USpringArmComponent::SocketName);
	
	//First Person Camera settings
	FirstPersonSpringArm->TargetArmLength = 100.f;
	FirstPersonCamera->FieldOfView = 110.f;
	FirstPersonSpringArm->bUsePawnControlRotation = false;
	FirstPersonCamera->bUsePawnControlRotation = true;
	FirstPersonSpringArm->SocketOffset = FVector(155.0f, 0.0f, 54.0f);
	
	//Debug Camera Component

	DebugCamera = CreateDefaultSubobject<UCameraComponent>(FName("DebugCamCamera"));
	DebugSpringArm = CreateDefaultSubobject<USpringArmComponent>(FName("Debug Spring Arm"));
		
	DebugCamera->bUsePawnControlRotation = false;
	DebugSpringArm->bUsePawnControlRotation = false;
	DebugSpringArm->SetupAttachment(RootComponent);
	DebugCamera->SetupAttachment(DebugSpringArm,USpringArmComponent::SocketName);
	DebugSpringArm->AddWorldRotation(FRotator(-90.0f,0.0f,0.0f));
	DebugCamera->AddWorldRotation(FRotator(0.0f,0.0f,0.0f));
	
	
	//Debug CAM SETTINGS
	DebugCamera->FieldOfView = 102.f;
	DebugSpringArm->TargetArmLength = 1600.f;
	
	if (bDevMode == false)
	{
		FirstPersonCamera->Activate();
		DebugCamera->Deactivate();
	}
	
	PlayerDeathHeight = 14900.f;
}



// Called when the game starts or when spawned
void APlayerCharacter::BeginPlay()
{
	Super::BeginPlay();
	//TODO:General Switch about use Source Movement or origin UE Movement
}

// Called every frame
void APlayerCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	if (SpeedShowingSwitch==true)
	{
		ShowVelocity();
	}
	
	if (GetActorLocation().Z < PlayerDeathHeight)
	{
		Respawn();
	}
}

// Called to bind functionality to input
void APlayerCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		//Look Trigger
		EnhancedInputComponent->BindAction(LookAction,ETriggerEvent::Triggered,this,&APlayerCharacter::MouseLookInput);
		
		//Move Trigger
		EnhancedInputComponent->BindAction(MoveAction,ETriggerEvent::Triggered,this,&APlayerCharacter::MoveInput);	
		
		//RouteB fix: Enhanced Input fires Completed on release, never a zero-value Triggered
		EnhancedInputComponent->BindAction(MoveAction,ETriggerEvent::Completed,this,&APlayerCharacter::MoveInputReleased);
		EnhancedInputComponent->BindAction(MoveAction,ETriggerEvent::Canceled, this,&APlayerCharacter::MoveInputReleased);
		
		//Jump Trigger
		if (AutoBhopFunction)
		{
			EnhancedInputComponent->BindAction(JumpAction,ETriggerEvent::Triggered,this,&APlayerCharacter::DoJumpStart);
			EnhancedInputComponent->BindAction(JumpAction,ETriggerEvent::Completed,this,&APlayerCharacter::DoJumpEnd);
		}
		if (!AutoBhopFunction)
		{
			EnhancedInputComponent->BindAction(JumpAction,ETriggerEvent::Started,this,&APlayerCharacter::DoJumpStart);
			EnhancedInputComponent->BindAction(JumpAction,ETriggerEvent::Completed,this,&APlayerCharacter::DoJumpEnd);
		}
		
		EnhancedInputComponent->BindAction(DebugAction,ETriggerEvent::Triggered,this,&APlayerCharacter::DebugMenuCalled);
	}
}

void APlayerCharacter::MoveInput(const FInputActionValue& Value)
{
	if (bDevMode)
	{
		GEngine->AddOnScreenDebugMessage(-1 , 0.0f, FColor::Green,"Input Detctive");
	}
	
	FVector2D Vector2D = Value.Get<FVector2D>();

	//RouteB fix: send the scalars straight to the movement component instead of AddMovementInput
	if (USourceCharacterMovementComponent* SourceMove = Cast<USourceCharacterMovementComponent>(GetCharacterMovement()))
	{
		SourceMove->SetMovementInput(Vector2D.Y, Vector2D.X); //Y = forward, X = side
	}
}

void APlayerCharacter::MoveInputReleased()
{
	ClearSourceMovementInput();
}

void APlayerCharacter::ClearSourceMovementInput()
{
	if (USourceCharacterMovementComponent* SourceMove = Cast<USourceCharacterMovementComponent>(GetCharacterMovement()))
	{
		SourceMove->SetMovementInput(0.0f, 0.0f);
	}
}

void APlayerCharacter::MouseLookInput(const FInputActionValue& Value)
{
	if (bDevMode)
	{
		GEngine->AddOnScreenDebugMessage(-1 , 0.0f, FColor::Green,"Mouse Spining Detective");
	}
	FVector2D V = Value.Get<FVector2D>();
	
	DoLook(V.X,V.Y);
}

void APlayerCharacter::DebugMenuCalled(const FInputActionValue& Value)
{
	TObjectPtr<APlayerController> roller = GetController<APlayerController>(); 
	
	if (!roller)
	{
		return; 
	}
	
	if (DebugWidgetInstance && DebugWidgetInstance->IsInViewport())
	{
		DebugWidgetInstance->RemoveFromParent();
		roller->bShowMouseCursor = false;
		roller->SetInputMode(FInputModeGameOnly());
		//RouteB fix: input was ignored while the panel was up, the release event got swallowed
		ClearSourceMovementInput();
	}
	
	//RenderDebug HUD
	if (bDevMode != true)
	{
		GEngine->AddOnScreenDebugMessage(1, 1.f, FColor::Emerald, FString(TEXT("Sv_DevMode is not true")));
		return;
	}
	else
	{
		GEngine->AddOnScreenDebugMessage(1, 1.f, FColor::Cyan, FString(TEXT("Sv_DevMode is true")));
		if (!DebugWidgetInstance)
		{
			DebugWidgetInstance = CreateWidget<UUserWidget>(roller,DebugUI);
		}
		if (DebugWidgetInstance)
		{
			DebugWidgetInstance->AddToViewport();
			roller->SetShowMouseCursor(true);
			roller->SetInputMode(FInputModeUIOnly());
			//RouteB fix: UIOnly makes the viewport ignore input, so stop here
			ClearSourceMovementInput();
		}
	}
	
}

void APlayerCharacter::Domove(float right, float left)
{
	if (GetController())
	{
		AddMovementInput(GetActorRightVector(), right);
		AddMovementInput(GetActorForwardVector(), left);
	}
	
	//Version 2
	/*if (!Controller)
	{
		return;
	}
	
	const FRotator ControlRotation = Controller->GetControlRotation();
	const FRotator YawRotation(0.0f, ControlRotation.Yaw, 0.0f);

	const FVector ForwardDirection =
		FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
	const FVector RightDirection =
		FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

	AddMovementInput(ForwardDirection, left);
	AddMovementInput(RightDirection, right);*/
}

void APlayerCharacter::DoLook(float Yaw, float Pitch)
{
	if (GetController())
	{
		AddControllerYawInput(Yaw);
		AddControllerPitchInput(Pitch);
	}
}

void APlayerCharacter::DoJumpStart()
{
	Jump();
}

void APlayerCharacter::DoJumpEnd()
{
	StopJumping();
}

void APlayerCharacter::ShowVelocity()
{
	FVector Velocity = GetVelocity();

	//RouteB fix: Acceleration is always 0 now, print the scalar channel instead
	if (const USourceCharacterMovementComponent* SourceMove = Cast<USourceCharacterMovementComponent>(GetCharacterMovement()))
	{
		FVector WishDir = FVector::ZeroVector;
		float WishSpeed = 0.0f;
		SourceMove->CalcWishDirAndSpeed(WishDir, WishSpeed);
		GEngine->AddOnScreenDebugMessage(-1, 0.0f, FColor::Green, FString::Printf(
			TEXT("mv: forward=%.3f side=%.3f"), SourceMove->GetForwardMove(), SourceMove->GetSideMove()));
		GEngine->AddOnScreenDebugMessage(-1, 0.0f, FColor::Green, FString::Printf(
			TEXT("wishdir: X=%.3f Y=%.3f | wishspeed=%.1f"), WishDir.X, WishDir.Y, WishSpeed));
	}

	GEngine->AddOnScreenDebugMessage(-1 , 0.0f, FColor::Green,FString::Printf(TEXT("MaxWalkSpeed: %f"), GetCharacterMovement()->MaxWalkSpeed));
	GEngine->AddOnScreenDebugMessage(-1 , 0.0f, FColor::Green,FString::Printf(TEXT("Speed: %f %f %f Size2D:%f"), Velocity.X,Velocity.Y,Velocity.Z,Velocity.Size2D()));
}

void APlayerCharacter::SwitchToFirstCam()
{
	if (FirstPersonCamera && DebugCamera)
	{
		FirstPersonCamera->Activate();
		DebugCamera->Deactivate();		
	}
	
}

void APlayerCharacter::SwitchToThirdCam()
{
	if (FirstPersonCamera && DebugCamera)
	{
		FirstPersonCamera->Deactivate();
		DebugCamera->Activate();
	}
	
}


void APlayerCharacter::Respawn()
{
	AActor *TargetSP = CurrentSpawnPoint;
	if (!IsValid(TargetSP))
	{
		TargetSP = UGameplayStatics::GetActorOfClass(this, APlayerStart::StaticClass());
	}
	if (!IsValid(TargetSP))
	{
		return;
	}
	UE_LOG(LogTemp, Warning, TEXT("[Player] Respawn() -> %s @ %s"),
		*GetNameSafe(TargetSP), *TargetSP->GetActorLocation().ToString());

	GetCharacterMovement()->StopMovementImmediately();
	GetCharacterMovement()->SetMovementMode(MOVE_Walking);
	SetActorLocationAndRotation(TargetSP->GetActorLocation(), TargetSP->GetActorRotation(),false,nullptr,ETeleportType::TeleportPhysics);
}

void APlayerCharacter::SetSpawnPoint(AActor* InPoint)
{
	CurrentSpawnPoint = InPoint;
	// 调试: DevMode 下打印新重生点
	if (bDevMode && InPoint)
	{
		GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Yellow,
			FString::Printf(TEXT("[Player] 重生点已更新: %s @ %s"),
				*InPoint->GetName(), *InPoint->GetActorLocation().ToString()));
	}
}

//TODO: Fearless Yuzuki Rebuild Version 2 
