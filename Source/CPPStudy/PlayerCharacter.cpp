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
	
	AWorldSettings *p_worldSettings = GetWorld()->GetWorldSettings();
	if (p_worldSettings)
	{
		float f_killZheight = p_worldSettings->KillZ;
		if (GetActorLocation().Z < f_killZheight)
		{
			Respawn();
		}
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
		
		/* ────────────────────────────────────────────────────────────────────────
		 * [新代码 02] 路线 B 必需的第二半：松手清零（与 JumpAction 的
		 *   Completed → DoJumpEnd 是同一套 idiom）。
		 *
		 * 不绑这两个事件的后果：IA_Move 用的是隐式 UInputTriggerDown，
		 * 松开时它返回 None，于是「松手」只会派发 Completed，
		 * 永远不会派发一个值为 0 的 Triggered → MoveInput 不再被调用 →
		 * 组件里的 mv_forwardMove/mv_sideMove 就一直保持最后的值（粘住）→
		 * ApplyFriction 刚减速，WalkMove 的 Accelerate 立刻把速度加回来 →
		 * 表现为「地面上松手后还在走，永远停不下来」。
		 *
		 * 为什么改前（路线 A）不需要这两行：AddMovementInput 只往
		 * ControlInputVector 里累积，UE 每 tick 会在 TickComponent 里
		 * ConsumeInputVector() 清空它 —— 「输入是每 tick 的命令」这个语义
		 * 原是引擎兜住的。路线 B 自己保管标量，就得自己兜（Source 也是每帧
		 * 用当前按键状态重建 usercmd，松手即为 0，不存在粘滞）。
		 * ──────────────────────────────────────────────────────────────────────── */
		EnhancedInputComponent->BindAction(MoveAction,ETriggerEvent::Completed,this,&APlayerCharacter::MoveInputReleased);
		// Canceled 一起绑：现用隐式 Down trigger 只会走 Completed；将来若换成
		// Hold/Pulse 之类，未达成即释放会走 Canceled，先绑上免得再踩同一个坑。
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

	/* ────────────────────────────────────────────────────────────────────────
	 * [新代码 01] 路线 B：标量直送移动组件
	 *   笔记第六节路线 B 的接线：
	 *       Move->SetMovementInput(V.Y, V.X);   // forward, side —— 按你的键位映射对齐
	 *   原路径 Domove → AddMovementInput 会把「基 × 标量」提前压成一个世界向量，
	 *   标量的独立身份就没了；现在标量一路保持到 CalcWishDirAndSpeed() 内部，
	 *   合成点每 tick 读「当帧」GetControlRotation() → wishdir 帧时序错位结构性消失。
	 *   形参顺序为 SetMovementInput(ForwardIn, SideIn)，故 forward = Y、side = X
	 *   （与改前 Domove(X=right, Y=left) 的轴向约定完全一致，键位手感不变）。
	 * ──────────────────────────────────────────────────────────────────────── */
	if (USourceCharacterMovementComponent* SourceMove = Cast<USourceCharacterMovementComponent>(GetCharacterMovement()))
	{
		SourceMove->SetMovementInput(Vector2D.Y, Vector2D.X);
	}

	/* ======================== [旧代码分片 06] ========================
	 * MoveInput 原实现：交给 Domove → AddMovementInput 走 UE 输入链（路线 A）。
	 * 保留原因：可随时切回旧路径做 A/B 手感对比。
	 * 恢复方法：注释掉上方 [新代码 01] 块，取消本块注释。
	 * ----------------------------------------------------------------
	Domove(Vector2D.X,Vector2D.Y);
	 * ===============================================================*/
}

void APlayerCharacter::MoveInputReleased()
{
	/* ────────────────────────────────────────────────────────────────────────
	 * [新代码 02] 路线 B 的「松手清零」—— 少了它就是「地面停不下来」
	 *
	 * 根因（已对着 UE 5.7 引擎源逐条确认，不是猜的）：
	 *   ① IA_Move 没有显式 trigger，用的是隐式 UInputTriggerDown，而它在松开时
	 *      返回 ETriggerState::None
	 *      （InputTriggers.cpp:132-135 return IsActuated(ModifiedValue) ? Triggered : None）；
	 *   ② 事件派发只在「本 tick 的 TriggerEvent == 绑定时指定的事件」时才调回调
	 *      （EnhancedPlayerInput.cpp:631 if (ActionData->TriggerEvent == BoundTriggerEvent)）；
	 *   ③ 于是松手只会派发 Completed，永远不会派发一个值为 0 的 Triggered，
	 *      MoveInput 不再被调用 → 组件里的标量保持最后的值（粘住）。
	 *
	 * 症状链：粘住后 ApplyFriction 照常在减速，但紧随其后的 WalkMove→Accelerate
	 *   里 addspeed = wishSpeed - Velocity·wishdir 仍 > 0，速度立刻被加回去，
	 *   所以表现为「松手后一直走 / 地面永远停不下来」。
	 *   —— 这与摩擦力公式无关：ApplyFriction / Sv_Friction / Sv_StopSpeed
	 *      本次重构一行都没动。
	 *
	 * 这是路线 B 的语义代价：路线 A 时「每 tick 一条新命令」由 UE 的
	 *   ConsumeInputVector() 兜住（AddMovementInput 只累积，TickComponent 清空），
	 *   自己保管标量就必须自己重建这个语义 —— Source 侧也一样，
	 *   CInput::CreateMove 每帧用当前按键状态重建 usercmd，松手即为 0。
	 *
	 * 调试提示：若以后又出现「停不下来」，先看屏幕上的 mv: f/s 是否卡在非 0，
	 *   卡住就说明这条清零路径没生效（事件绑定被删/被改），而不是摩擦算错了。
	 * ──────────────────────────────────────────────────────────────────────── */
	ClearSourceMovementInput();
}

void APlayerCharacter::ClearSourceMovementInput()
{
	// [新代码 03] 路线 B 标量清零的唯一入口。
	// 调用点：MoveInputReleased（松手）、DebugMenuCalled（切输入模式）。
	// 之所以要抽成一个函数，是因为路线 B 下「什么时候必须主动清零」有两类：
	//   ① 按键状态变化（松手）—— 事件能收到，用 Completed/Canceled 绑；
	//   ② 输入被整体切断（FInputModeUIOnly 让视口 SetIgnoreInput(true)，
	//      见 PlayerController.cpp:6260）—— 事件根本收不到，只能在切换点主动清。
	//      不主动清的话：按住 W 开调试面板 → 面板里松开 W → 关面板 →
	//      组件里的标量还停在被忽略之前的 1.0，角色自己往前走。
	// 路线 A（AddMovementInput + 引擎每 tick ConsumeInputVector）这两类都由引擎兜住，
	// 这也是自己保管标量的代价。
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
		// [新代码 03] 面板期间视口忽略输入，松开按键的事件被吞掉了，回来时主动清零，
		// 否则关面板后角色会带着面板打开前的输入继续走。
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
			// [新代码 03] UIOnly 会让视口 SetIgnoreInput(true)（PlayerController.cpp:6260），
			// 之后按键事件不再送到 UEnhancedPlayerInput，标量会冻结在此刻的值。
			// 开面板时先清零：既是避免粘住，也符合「开调试面板时角色停下」的预期。
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
	
	/* ────────────────────────────────────────────────────────────────────────
	 * [路线B 说明] 本函数在路线 B 下已不在输入主链上（MoveInput 不再调用它）。
	 *   函数体保持原样、未改动，以便随时切回路线 A 对照。
	 *
	 *   ★ 已知差异（笔记第五节事实 #1、第八节「不要混用两个基」）：
	 *     这里用的是 Actor 基（GetActorRightVector / GetActorForwardVector），
	 *     而 Source 用的是视角 Yaw 基。现在「Actor 基 ≈ Yaw 基」只是因为
	 *     ACharacter 默认 bUseControllerRotationYaw = true 的巧合 ——
	 *     而全工程从未显式设置过它（正确性不该建立在没写出来的默认值上）。
	 *     下面注释里的 Version 2 才是 Source 做法；路线 A 复现时启用它即可，
	 *     但注意不要「Domove 用 Actor 基 + 反投影用 View 基」混用，
	 *     否则反投影出的标量会随「视角-朝向夹角」漂移成 cos 值而不是 ±1。
	 * ──────────────────────────────────────────────────────────────────────── */

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

	/* [新] 路线 B 下 UE 的 Acceleration 恒为 0（不再有 AddMovementInput →
	 * 不再有 ScaleInputAcceleration），调试读数改走标量通道与合成结果。 */
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

	/* ======================== [旧代码分片 07] ========================
	 * ShowVelocity 原实现：打印 UE 的 CurrentAcceleration。
	 * 保留原因：路线 B 下它恒为 0，但切回路线 A 时仍是有效读数。
	 * 恢复方法：注释掉上方 [新] 打印块，取消本块注释。
	 * ----------------------------------------------------------------
	double Acc = GetCharacterMovement()->GetCurrentAcceleration().Size2D();
	GEngine->AddOnScreenDebugMessage(-1 , 0.0f, FColor::Green,FString::Printf(TEXT("Crt Acceleration: %f"),Acc));
	 * ===============================================================*/

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

