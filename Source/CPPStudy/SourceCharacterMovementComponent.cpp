// Fill out your copyright notice in the Description page of Project Settings.

#include "SourceCharacterMovementComponent.h"
#include "GameFramework/Character.h"
#include "Camera/CameraComponent.h"   // 仍被 [旧代码分片 05] 的「相机版」调试线引用（取消注释即可用）
#include "PlayerCharacter.h"
#include "Engine/Engine.h"
#include "Math/RotationMatrix.h"      // [路线B] FRotationMatrix —— 构造视角 Yaw 基向量对


USourceCharacterMovementComponent::USourceCharacterMovementComponent()
{
	//H3 fix: one-time init moved out of CalcVelocity. Runtime values are owned by UPROPERTYs.
	GroundFriction = 8.f;
	MaxAcceleration = 600.f;
	
	// [路线B 说明] MaxAcceleration 在路线 B 下不再参与输入合成（原来它经
	// UE 的 ScaleInputAcceleration 把标量变成 Acceleration）。保留不动，未删除。
	
	JumpZVelocity = 600.f;
}

/* ════════════════════════════════════════════════════════════════════════════
 *  [新代码 01] 标量通道 + Source 式合成（路线 B 的核心）
 * ════════════════════════════════════════════════════════════════════════════ */

void USourceCharacterMovementComponent::SetMovementInput(float ForwardIn, float SideIn)
{
	// = Source usercmd_t::m_flForwardMove / m_flSideMove 的写入点
	//   （重构前：全工程零调用，二者恒为 0 —— 现状事实 #2）
	// 本函数自建立起一直是这个「单轴钳幅」实现，未改动，故不另设分片注释。
	mv_forwardMove = FMath::Clamp(ForwardIn, -1.0f, 1.0f);
	mv_sideMove = FMath::Clamp(SideIn, -1.0f, 1.0f);

	// ★ 调用契约：本通道是「组件持有当前按键状态」，不是「每 tick 累加」。
	//   输入层必须在按键变化的两侧都调用本函数：Triggered 写当前值，
	//   Completed / Canceled 写 (0,0)。Enhanced Input 松手只派发 Completed，
	//   不会派发值为 0 的 Triggered —— 漏了清零，标量就会粘住，
	//   于是 ApplyFriction 刚减速就被 WalkMove 的 Accelerate 加回去，
	//   表现为「松手后一直走、地面停不下来」。见 PlayerCharacter::MoveInputReleased()。

	// 注：这里只做单轴 [-1,1] 钳制。「斜向不放大」的模长钳制
	// （Source CheckParameters，gm.cpp:992-1036）放在 CalcWishVel() 里做，
	// 与 Source 一致 —— 它同样是在合成前后各钳一次（gm.cpp:992 与 1781）。
}

void USourceCharacterMovementComponent::GetViewBasisVectors(FVector& OutForward, FVector& OutRight) const
{
	/* = Source AngleVectors(mv->m_vecViewAngles, &forward, &right, &up)
	 *   + forward[2] = 0; right[2] = 0; VectorNormalize(...)   (gm.cpp:1760-1771)
	 *
	 * 视角的真相源是 Controller 旋转，而不是某个 CameraComponent 的旋转：
	 * 第三人称 SpringArm 摆动时相机会被甩，控制器旋转才是稳的
	 * （PBCharacterMovement.cpp:584 GetCameraRoll 就是同款做法）。
	 *
	 * FRotationMatrix(FRotator(0, Yaw, 0)) 的 X / Y 轴天然 Z == 0 且已是单位向量，
	 * 等价于 Source「先取 forward/right 再把 Z 清零重归一化」的结果，
	 * 也就是只留 Yaw 旋转矩阵的两个水平轴 —— 俯仰不参与地面/空中移动。 */
	const float ViewYaw = CharacterOwner ? CharacterOwner->GetControlRotation().Yaw : 0.0f;
	const FRotator YawRotation(0.0f, ViewYaw, 0.0f);
	const FMatrix ViewBasis = FRotationMatrix(YawRotation);

	OutForward = ViewBasis.GetUnitAxis(EAxis::X);   // Source 的 forward（Z == 0）
	OutRight   = ViewBasis.GetUnitAxis(EAxis::Y);   // Source 的 right  （Z == 0）
}

FVector USourceCharacterMovementComponent::CalcWishVel() const
{
	/* = Source 的 wishvel 合成，三步都对着 gamemovement.cpp 的行号：
	 *   [1] CheckParameters 式输入钳幅（gm.cpp:992-1036）
	 *   [2] wishvel = forward × fmove + right × smove（gm.cpp:1773）
	 *   [3] wishvel[2] = 0（gm.cpp:1773 末行） */

	// [1] 输入标量先钳幅：斜向输入（W+D，(1,1)，模长 √2）不会被放大
	//     等价于 UE 的 GetClampedToMaxSize(1)（UE:8018 ScaleInputAcceleration）。
	//     ★ 路线 B 自己合成时最容易漏的就是这一步，漏了斜跑会比直跑快 √2 倍。
	float F = mv_forwardMove;
	float S = mv_sideMove;
	const float InputSpdSq = F * F + S * S;   // UpMove（m_flUpMove）本项目未使用，恒 0
	if (InputSpdSq > 1.0f)
	{
		const float Ratio = 1.0f / FMath::Sqrt(InputSpdSq);
		F *= Ratio;
		S *= Ratio;
	}

	// [2] 用「视角基向量对」合成 —— 不是角度、也不是单根视线方向
	FVector ViewForward, ViewRight;
	GetViewBasisVectors(ViewForward, ViewRight);
	FVector WishVel = ViewForward * F + ViewRight * S;

	// [3] 合成结果恒为水平（forward/right 的 Z 已是 0，这里再显式保证一次）
	WishVel.Z = 0.0f;

	return WishVel;
}

void USourceCharacterMovementComponent::CalcWishDirAndSpeed(FVector& OutWishDir, float& OutWishSpeed) const
{
	/* = Source AirMove / WalkMove 里那两行（gm.cpp:1781-1786）：
	 *     wishspeed = VectorNormalize(wishdir);
	 *     if (wishspeed != 0 && wishspeed > mv->m_flMaxSpeed) { ... scale down ... }
	 *   方向 wishdir 与速度标量 wishspeed 是「同一次归一化」的两个产物。 */

	const FVector WishVel = CalcWishVel();
	const float WishSpeedScalar = WishVel.Size();   // Z 已为 0，等价 Size2D；含 [1] 的钳幅 → ∈[0, 1]

	// 方向：GetSafeNormal() 在零向量时返回 ZeroVector，天然安全
	OutWishDir = WishVel.GetSafeNormal();

	// [4] 标量 [-1,1] → 速度量级。
	//     本项目输入端就是归一化标量，所以 Source 的 cl_forwardspeed / cl_sidespeed
	//     这两级被折叠进 MaxGroundSpeed：单按 W 即达到 maxspeed。
	//     若要严格复刻 Source「450 输入 / 320 maxspeed」的量级差，在此乘一个
	//     Cl_ForwardSpeed(450) / MaxSpeed 的比例即可，结构不用动。
	OutWishSpeed = WishSpeedScalar * MaxGroundSpeed;

	// [5] 合成后再钳一次（gm.cpp:1781）：wishspeed 永远 ≤ mv->m_flMaxSpeed。
	//     与 [1] 构成 Source 的「双钳制」。此处看似冗余（[1] 已保证标量 ≤ 1），
	//     但保留它以维持与 gm.cpp 的行级对照，且防住 MaxGroundSpeed 被运行时改小的情形。
	if (OutWishSpeed > MaxGroundSpeed)
	{
		OutWishSpeed = MaxGroundSpeed;
	}
}

/* ════════════════════════════════════════════════════════════════════════════
 *  [新代码 02] AirMove —— 改由 CalcWishDirAndSpeed() 取 wishdir / wishspeed
 * ════════════════════════════════════════════════════════════════════════════ */

void USourceCharacterMovementComponent::AirMove(float DeltaTime)
{
	FVector wishdir = FVector::ZeroVector;
	float wishSpeed = 0.0f;
	CalcWishDirAndSpeed(wishdir, wishSpeed);   // = AngleVectors + 合成 + VectorNormalize + 钳制

	// 传进去的是「未钳到 Sv_AirCappingSpeed」的 wishSpeed。
	// Source 的 30 上限只作用于 addspeed（见 AirAcceleration），
	// 在调用方就钳掉会让 accelspeed 也用错值 —— 那正是重构前的 H2/P2 号问题。
	AirAcceleration(wishdir, wishSpeed, Sv_AirAcceleration, DeltaTime);
}

// ======================== [旧代码分片 01] ========================
// AirMove 原实现（路线 A：读 UE 的 Acceleration；wishspeed 在调用方就被 30 钳死）
// 保留原因：可回退的 A/B 手感对照。这段也正是「Sv_AirAcceleration 被写成 100」的成因：
//           钳到 30 后为了让 accelspeed 有量级，只能把 accel 放大到 100 倍。
// 恢复方法：注释掉上方 [新代码 02] 块，去掉本块每行行首的 //，
//           并把 .h 里 Sv_AirAcceleration 从 10 调回 100。
// 承载方式说明：本块整体用「行注释」承载，因此它内部原有的块注释可以保持与源文件
//   逐字一致（若改用块注释承载，内嵌的结束符会提前闭合，就必须改写 —— 那才是有损的）。
// ----------------------------------------------------------------
//void USourceCharacterMovementComponent::AirMove(float DeltaTime)
//{
//	/*if (GEngine)
//	{
//		GEngine->AddOnScreenDebugMessage(-1, 0, FColor::Blue,TEXT("Source air calc input"));
//	}*/
//	FVector wishdir = Acceleration.GetSafeNormal2D();
//	float wishSpeed = Acceleration.Size2D();
//	float acceleration = Sv_AirAcceleration;
//	
//	if (wishSpeed > Sv_AirCappingSpeed)
//	{
//		wishSpeed = Sv_AirCappingSpeed;
//	}
//	
//	AirAcceleration(wishdir,wishSpeed,acceleration,DeltaTime);
//}
// ===============================================================

/* ════════════════════════════════════════════════════════════════════════════
 *  [新代码 03] AirAcceleration —— 把 30 的 cap 挪回它该在的位置
 * ════════════════════════════════════════════════════════════════════════════ */

void USourceCharacterMovementComponent::AirAcceleration(FVector wishdir, float wishSpeed, float acceleration,
                                                        float DeltaTime)
{
	/* = Source AirAccelerate（gm.cpp:1711-1738）。三个要点：
	 *   ① 速度上限 30 只钳「addspeed 用的副本」wishspd，不钳 accelspeed 用的 wishspeed；
	 *   ② accelspeed = accel × wishspeed × dt × surfaceFriction（空中 surfaceFriction = 1）；
	 *   ③ addspeed ≤ 0 直接返回（沿 wishdir 已没有可加空间）。 */
	if (!IsFalling())
	{
		return;
	}

	if (wishSpeed <= 0.0f || wishdir.IsNearlyZero())
	{
		// 无输入时的等价提前返回：Source 里 wishspeed = 0 → addspeed = 0 - 0 = 0 → 同样 return
		return;
	}

	const float WishSpd = FMath::Min(wishSpeed, Sv_AirCappingSpeed); // ★ cap 只作用于 addspeed

	float addSpeed = 0.0f;//init
	const float crtspeed = Velocity.Dot(wishdir);
	float accelspeed = 0.0f;//init

	addSpeed = WishSpd - crtspeed;

	if (addSpeed <= 0)
	{
		return;
	}

	accelspeed = wishSpeed * DeltaTime * acceleration; // ★ 未钳制的 wishSpeed（gm.cpp:1734）

	if (accelspeed > addSpeed)
	{
		accelspeed = addSpeed;
	}

	Velocity += accelspeed * wishdir;
}

/* ======================== [旧代码分片 02] ========================
 * AirAcceleration 原实现：cap 已由调用方钳进 wishSpeed，addspeed 与 accelspeed
 * 共用这一个（被钳到 30 的）wishSpeed —— 与 gm.cpp:1720-1734 的差异点，即 H2/P2。
 * 保留原因：行级差异对照（面试 / 博客都要用）。
 * 恢复方法：注释掉上方 [新代码 03] 块，取消本块注释。
 * ----------------------------------------------------------------
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
 * ===============================================================*/

/* ════════════════════════════════════════════════════════════════════════════
 *  [新代码 04] WalkMove —— 与 AirMove 同构（gm.cpp:1893-1959）
 * ════════════════════════════════════════════════════════════════════════════ */

void USourceCharacterMovementComponent::WalkMove(float DeltaTime)
{
	FVector wishdir = FVector::ZeroVector;
	float wishSpeed = 0.0f;
	CalcWishDirAndSpeed(wishdir, wishSpeed);   // 已含 wishspeed ≤ mv->m_flMaxSpeed（gm.cpp:1959）

	if (Velocity.Z != 0)
	{
		Velocity.Z = 0;
	}

	//TODO:Chara move like slide and if chara sped = 0 cant accel
	//TODO: Figure out what the hell UE controls Chara move and Figure Source Engine
	//（以上两条 TODO 为原有注释，保留）

	Accelerate(wishdir, wishSpeed, Sv_Accelerate, DeltaTime);
}

/* ======================== [旧代码分片 03] ========================
 * WalkMove 原实现：wishdir/wishspeed 取自 UE 的 Acceleration；另有恒为 true 的
 * bCanAccele 开关（两个分支等价，属死逻辑）。
 * 保留原因：加速度来源（Pawn 层合成 vs 组件内合成）的对照。
 * 恢复方法：注释掉上方 [新代码 04] 块，取消本块注释。
 * ----------------------------------------------------------------
void USourceCharacterMovementComponent::WalkMove(float DeltaTime)
{
	FVector wishdir = Acceleration.GetSafeNormal2D();
	float wishSpeed = FMath::Min(Acceleration.Size2D(), MaxGroundSpeed); //H1 fix: wishspeed clamped to maxspeed, like Source WalkMove
	float acceleration = Sv_Accelerate; //H1 fix: sv_accelerate scalar, not a velocity magnitude
	bool bCanAccele;
	
	if (Velocity.Z != 0)
	{
		Velocity.Z = 0;
	}
	
	bCanAccele = true; //Can Accel Function has More Ristriction
	
	//TODO:Chara move like slide and if chara sped = 0 cant accel
	//TODO: Figure out what the hell UE controls Chara move and Figure Source Engine
	
	if (bCanAccele)
	{
		Accelerate(wishdir,wishSpeed,acceleration,DeltaTime);
	}
	
	if (!bCanAccele)
	{
		return;
	}
	
}
 * ===============================================================*/

/* ════════════════════════════════════════════════════════════════════════════
 *  Accelerate / ApplyFriction —— 本次重构未改动（P3/P4 修正后的实现已是 Source 形状）
 *  仅补充行号对照注释，函数体保持原样。
 * ════════════════════════════════════════════════════════════════════════════ */

void USourceCharacterMovementComponent::Accelerate(FVector wishdir, float wishSpeed, float acceleration,
														 float DeltaTime)
{
	// = Source Accelerate（gm.cpp:1820-）。传入的 wishSpeed 来自 CalcWishDirAndSpeed()，
	//   即经过 CheckParameters(gm.cpp:992) + 合成后钳制(gm.cpp:1781) 双钳制的值。
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
	// = Source ApplyFriction / Friction（sv_friction / sv_stopspeed，gm.cpp 摩擦段）
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
	// [路线B] 调试区改读「标量通道 + 合成结果」：Acceleration 在路线 B 下恒为 0，
	//          继续画它只会得到一条零长度的线。
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

			// 黄线 = 合成出来的 wishvel（= wishdir × wishspeed 的方向与量级）
			DrawDebugLine(GetWorld(),GetActorLocation(),GetActorLocation() + CalcWishVel() * DebugLineLength,FColor::Yellow,false,-1.0f,0,3.0f);
			// 红线 = 当前速度方向
			DrawDebugLine(GetWorld(),GetActorLocation(),GetActorLocation() + Velocity.GetSafeNormal2D() * 300.0f,FColor::Red,false,-1.0f,0,3.0f);

			GEngine->AddOnScreenDebugMessage(-1,0.0f,FColor::Yellow,FString::Printf(
				TEXT("mv: f=%.3f s=%.3f | wishdir: X=%.3f Y=%.3f | wishspeed=%.1f | vel=%.1f"),
				mv_forwardMove, mv_sideMove, WishDir.X, WishDir.Y, WishSpeed, Velocity.Size2D()));
		}
	}

	/* ======================== [旧代码分片 04] ========================
	 * CalcVelocity 调试区原实现：画 Acceleration 方向线 / 打印 Acceleration 分量。
	 * 保留原因：Acceleration 语义在路线 B 下失效，但切回路线 A 时这段依然可用。
	 * 恢复方法：注释掉上方 [路线B] 调试块，取消本块注释。
	 * ----------------------------------------------------------------
	DrawDebugLine(GetWorld(),GetActorLocation(),GetActorLocation()+Acceleration.GetSafeNormal2D(),FColor::Green,false,-1,0,3.0f);
	DrawDebugLine(GetWorld(),GetActorLocation(),GetActorLocation() + Velocity.GetSafeNormal2D() * 300.0f,FColor::Red,false,-1.0f,0,3.0f);

	FVector InputAcceleration = Acceleration;

	GEngine->AddOnScreenDebugMessage(-1,0.0f,FColor::Yellow,FString::Printf(TEXT("Acceleration: X=%f Y=%f Z=%f Size2D=%f")
		,InputAcceleration.X,InputAcceleration.Y,InputAcceleration.Z,InputAcceleration.Size2D()));
	 * ===============================================================*/
	
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
	// 遗留：ApplyFriction 的 P4 修正后改为直接读 Sv_Friction，本函数已无调用点。保留，未删除。
	return GroundFriction;
}

void USourceCharacterMovementComponent::CalcVector(const FRotator &Angles, float Forward)
{
	// 遗留空桩，保留未删除。路线 B 的实现请看上方的
	// CalcWishVel() / CalcWishDirAndSpeed() / GetViewBasisVectors()。
}

/* ════════════════════════════════════════════════════════════════════════════
 *  [新代码 05] 调试线：一根 forward → 一对 Yaw 基向量 + 合成出的 wishvel
 * ════════════════════════════════════════════════════════════════════════════ */

void USourceCharacterMovementComponent::DrawCameraDebugline(bool bDevMode)
{
	APlayerCharacter* PlayerChar = Cast<APlayerCharacter>(CharacterOwner);

	// ★ 修补：原实现没有判空，CharacterOwner 不是 APlayerCharacter 时
	//   `PlayerChar->FindComponentByClass<>()` 会解引用空指针直接崩。
	if (!PlayerChar || !bDevMode)
	{
		return;
	}

	/* 笔记第四节：Source 要的是「基向量对」（forward 与 right），
	 * 只拿一根视线方向做不出 f×forward + s×right；而且必须丢掉 pitch
	 * （gm.cpp:1767-1771 明确 Z 清零）。这里用 Controller 旋转而不是
	 * 相机组件旋转：第三人称 SpringArm 摆动时相机朝向会被甩。 */
	FVector ViewForward, ViewRight;
	GetViewBasisVectors(ViewForward, ViewRight);
	const FVector Origin = GetActorLocation();

	// 绿线 = 视角 Yaw 基 X 轴（Source 的 forward，Z 已清零）
	DrawDebugLine(GetWorld(), Origin, Origin + ViewForward * DebugLineLength, FColor::Green,false,-1.0f,0,3.0f);
	// 蓝线 = 视角 Yaw 基 Y 轴（Source 的 right，Z 已清零）
	DrawDebugLine(GetWorld(), Origin, Origin + ViewRight   * DebugLineLength, FColor::Blue, false,-1.0f,0,3.0f);
	// 黄线 = forward × fmove + right × smove（合成结果，长度 ≤ √2 × DebugLineLength）
	DrawDebugLine(GetWorld(), Origin, Origin + CalcWishVel() * DebugLineLength, FColor::Yellow,false,-1.0f,0,3.0f);
}

/* ======================== [旧代码分片 05] ========================
 * DrawCameraDebugline 原实现：只画相机组件旋转出来的 forward 一根线。
 * 保留原因：这是「取相机旋转」那条路的原始证据（笔记第四节的猜想起点），留作对照。
 * 三个问题：① 含 pitch（Source 紧接着就 Z 清零重归一）
 *           ② 相机组件旋转在第三人称 SpringArm 摆动时会被甩
 *           ③ 没判空 → CharacterOwner 非 APlayerCharacter 时崩
 * 恢复方法：注释掉上方 [新代码 05] 绘制块，取消本块注释。
 * ----------------------------------------------------------------
void USourceCharacterMovementComponent::DrawCameraDebugline(bool bDevMode)
{
	APlayerCharacter* PlayerChar = Cast<APlayerCharacter>(CharacterOwner);
	UCameraComponent* CameraComp = PlayerChar->FindComponentByClass<UCameraComponent>();
	if (CameraComp && bDevMode)
	{
		FVector CameraLocation = CameraComp->GetComponentLocation();
		FRotator CameraRotation = CameraComp->GetComponentRotation();
		
		FVector CamFacing = CameraRotation.Vector().GetSafeNormal();
		
		DrawDebugLine(GetWorld(), GetActorLocation(),GetActorLocation()+CamFacing*DebugLineLength,FColor::Green,false,-1,0,3.0f);
	}
	else
	{
		return;
	}
	
}
 * ===============================================================*/
