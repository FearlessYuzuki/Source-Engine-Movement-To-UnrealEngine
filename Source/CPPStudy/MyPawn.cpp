// Fill out your copyright notice in the Description page of Project Settings.

#include "MyPawn.h"
#include "EnhancedInputSubsystems.h"
#include "EnhancedInputComponent.h"
#include "InputMappingContext.h"    
#include "GameFramework/CharacterMovementComponent.h"// 必须引入，才能识别 IMC 类型
#include "GameFramework/PlayerController.h" // 必须引入，才能使用 APlayerController 和 GetLocalPlayer()
#include "Engine/Engine.h"
#include "InputAction.h"

// Sets default values
AMyPawn::AMyPawn()
{
 	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void AMyPawn::BeginPlay()
{
	Super::BeginPlay();
}

// Called every frame
void AMyPawn::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

// Called to bind functionality to input
void AMyPawn::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	
	//GetIMC(Controller)
	if (APlayerController* PC = Cast<APlayerController>(GetController()))
	{
		if (ULocalPlayer* LocalPlayer = PC->GetLocalPlayer())
		{
			if (UEnhancedInputLocalPlayerSubsystem* LocalPlayerSubsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(LocalPlayer))
			{
				if (Input)
				{
					LocalPlayerSubsystem->AddMappingContext(Input, 0);
				}

			}
		}
	}
	  
	
	
	
	//BindAction
	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AMyPawn::MoveInput);
	}
}

void AMyPawn::MoveInput(const FInputActionValue& Value)
{
	FVector2D Movement2D = Value.Get<FVector2D>();
	DoMove(Movement2D.X, Movement2D.Y);
}


void AMyPawn::DoMove(float Right, float Forward)
{
	/* ────────────────────────────────────────────────────────────────────────
	 * [路线B 边界说明] —— 本次重构未改动本函数的逻辑，原因如下：
	 *
	 *   笔记第五节事实 #5：MyPawn 同样用 Actor 基，备用 Pawn 会复现同样的错位。
	 *   但路线 B 的形态是「标量存在移动组件里 + 合成在组件内部」，
	 *   而 AMyPawn 派生自 APawn（MyPawn.h:18），**根本没有 CharacterMovementComponent**，
	 *   没有任何地方能承载 mv_forwardMove / mv_sideMove —— 路线 B 在此结构上不适用。
	 *
	 *   两条可选后续（本次未做，避免在无法实机验证的情况下改动非主链代码）：
	 *     ① 若 MyPawn 只是实验/备用：改成 ACharacter 并挂
	 *        USourceCharacterMovementComponent，即可完全复用路线 B；
	 *     ② 若必须保持 APawn：把下面的 Actor 基换成视角 Yaw 基（路线 A 的思路）——
	 *        const FRotator YawRotation(0.0f, GetControlRotation().Yaw, 0.0f);
	 *        AddMovementInput(FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y), Right);
	 *        AddMovementInput(FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X), Forward);
	 *
	 *   现状：MyPawn 在全工程无任何引用（仅自身 .h/.cpp），不构成当前 bug，故先标注不动。
	 * ──────────────────────────────────────────────────────────────────────── */
	if (GetController())
	{
		AddMovementInput(GetActorRightVector(), Right);
		AddMovementInput(GetActorForwardVector(), Forward);
	}
}


