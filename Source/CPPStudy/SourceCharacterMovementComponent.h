// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "SourceCharacterMovementComponent.generated.h"
/**
 *  Idea come from Source SDK - 2013
 *
 *  ════════════════════════════════════════════════════════════════════════════
 *  [路线 B 重构] —— 「自己保管标量」
 *  依据笔记：《04 输入合成——从视角基向量 × WASD 到 wishvel》
 *  ════════════════════════════════════════════════════════════════════════════
 *  重构前（路线 A / UE 语义）：
 *      MoveInput → Domove → AddMovementInput(Actor 基, 标量)
 *      → PendingInputVector → Acceleration = MaxAcceleration × Clamp(InputVector, 1)
 *      → AirMove / WalkMove 读 Acceleration，wishdir = Acceleration.GetSafeNormal2D()
 *      ✗ 合成发生在 Pawn 层、用的是 Actor 基 → 视角 Yaw 与 wishdir 存在帧时序错位
 *      ✗ mv_forwardMove / mv_sideMove 声明了但全工程零调用、恒为 0
 *
 *  重构后（路线 B / Source 语义）：
 *      MoveInput → SetMovementInput(Forward, Side)          [纯标量，[-1,1]]
 *      → CalcWishDirAndSpeed()：视角 Yaw 基向量对 + Z 清零 + 合成 + 归一
 *      → AirMove / WalkMove 用 wishdir / wishspeed 调 AirAcceleration / Accelerate
 *      ✓ 合成点每 tick 读「当帧」GetControlRotation() → wishdir 时序错位结构性消失
 *      ✓ 不再经过 UE 的 GetFallingLateralAcceleration / AirControl 缩放链
 *
 *  ◆ 旧代码全部保留：被替换的函数体在 .cpp 内以「[旧代码分片 NN]」注释块存放，
 *    每块都写明了「保留原因 + 恢复方法」，取消注释即可切回做 A/B 手感对比。
 *  ◆ 参数命名对照（本项目 ↔ Source gamemovement.cpp）：
 *      MaxGroundSpeed  ≡ mv->m_flMaxSpeed（Source 的 WalkMove 与 AirMove 共用同一个值，
 *                        所以空中也复用它；名字里的 Ground 是历史遗留，未重命名是因为
 *                        MaxGroundSpeed 是 BlueprintReadWrite，改名会断蓝图引用）
 *      Sv_AirCappingSpeed ≡ GetAirSpeedCap()（默认 30，★只作用于 addspeed）
 *      Sv_AirAcceleration ≡ sv_airaccelerate
 *      Sv_Accelerate      ≡ sv_accelerate
 *      Sv_Friction        ≡ sv_friction
 *      Sv_StopSpeed       ≡ sv_stopspeed
 *      mv_forwardMove     ≡ usercmd_t::m_flForwardMove
 *      mv_sideMove        ≡ usercmd_t::m_flSideMove
 *  ════════════════════════════════════════════════════════════════════════════
 */
UCLASS()
class CPPSTUDY_API USourceCharacterMovementComponent : public UCharacterMovementComponent
{
	GENERATED_BODY()

public:
	USourceCharacterMovementComponent();	

	UPROPERTY(EditAnywhere, Category = "Tools|AutoBhoping")
	bool EnableBunnyhoping;
	virtual void CalcVelocity(float DeltaTime, float Friction, bool bFluid, float BrakingDeceleration) override;

	/* ────────────────────────────────────────────────────────────────────────
	 * [新] 路线 B 标量通道 —— Source usercmd_t 的等价物
	 * 由 APlayerCharacter::MoveInput 每帧把 Enhanced Input 的 FVector2D 拆成
	 * forward / side 两个标量喂进来（原来这套接口建好了但零调用）。
	 *
	 * ★ 调用契约（缺了就会「地面停不下来」）：本通道是「组件持有当前按键状态」
	 *   模型，输入层必须在按键变化的两侧都写 —— Triggered 写当前值，
	 *   Completed / Canceled 写 0。Enhanced Input 松手只派发 Completed，
	 *   不会派发值为 0 的 Triggered，漏掉清零标量就会粘住，
	 *   表现为摩擦被 Accelerate 立刻抵消、松手后停不下来。
	 *   见 PlayerCharacter.cpp 的 MoveInputReleased()。
	 * ──────────────────────────────────────────────────────────────────────── */
	void SetMovementInput(float ForwardIn, float SideIn);

	/** 合成结果 wishvel（已含 CheckParameters 式输入钳幅；Z 恒为 0）。
	 *  = ViewForward × fmove + ViewRight × smove，对应 gm.cpp:1773。 */
	UFUNCTION(BlueprintPure, Category = "Source Movement|WishVel")
	FVector CalcWishVel() const;

	/** Source `wishspeed = VectorNormalize(wishdir)` 的等价物：方向与速度标量一次算出，
	 *  对应 gm.cpp:1781-1786（含 wishspeed ≤ mv->m_flMaxSpeed 的钳制）。 */
	void CalcWishDirAndSpeed(FVector& OutWishDir, float& OutWishSpeed) const;

	/** 视角 Yaw 基向量对 = Source AngleVectors(viewangles) + forward[2]=right[2]=0 + 重归一
	 *  （gm.cpp:1760-1771）。取 Controller 旋转而非相机组件旋转：第三人称 SpringArm
	 *  摆动时相机朝向会被甩，控制器旋转才是视角的真相源（PBCharacterMovement:584 同款）。 */
	void GetViewBasisVectors(FVector& OutForward, FVector& OutRight) const;

	/** 供调试 UI / 蓝图读出的原始标量（等价于 usercmd 里的 m_flForwardMove / m_flSideMove） */
	UFUNCTION(BlueprintPure, Category = "Source Movement|WishVel")
	float GetForwardMove() const { return mv_forwardMove; }

	UFUNCTION(BlueprintPure, Category = "Source Movement|WishVel")
	float GetSideMove() const { return mv_sideMove; }

protected:
	
	UPROPERTY(EditAnywhere,Category="Move Settings")
	float Sv_AirAcceleration = 10.0f; //路线B: Source sv_airaccelerate 默认 10（100 → 10 的换算说明见 .cpp 内 [旧代码分片 01]）
	UPROPERTY(EditAnywhere,Category="Move Settings")
	float Sv_StopSpeed = 100.f; //H5 fix: Source sv_stopspeed default is 100, not 1	
	UPROPERTY(BlueprintReadWrite, EditAnywhere,Category="Move Settings")
	float MaxGroundSpeed = 600.f;
	UPROPERTY(EditAnywhere,Category="Move Settings")
	float Sv_Friction = 5.f;
	UPROPERTY(EditAnywhere,Category="Move Settings")
	float Sv_Accelerate = 12.f;
	UPROPERTY(EditAnywhere,Category="Move Settings")
	float Sv_AirCappingSpeed = 30.f;
	
	/* 路线 B 的标量：Source usercmd_t::m_flForwardMove / m_flSideMove（归一化，[-1,1]）。
	   重构前恒为 0（无人调用 SetMovementInput）；现在由 MoveInput 每帧写入。 */
	float mv_forwardMove = 0.f;
	float mv_sideMove = 0.f;
	float DebugLineLength = 500.f;
	
	
	/* 遗留：全工程无调用点。真正的空中 cap 是上面的 Sv_AirCappingSpeed（在 AirAcceleration 内使用）。
	   保留，未删除。 */
	virtual float GetCapppingAirAccleration() {return 100.0f;}
	
	void AirMove(float DeltaTime);
	
	void AirAcceleration(FVector wishdir, float wishSpeed, float acceleration, float DeltaTime);
	
	void WalkMove(float DeltaTime);
	
	void ApplyFriction(float DeltaTime);
	
	void Accelerate(FVector wishdir, float wishSpeed, float acceleration, float DeltaTime);
	
	FVector VectorScale(const FVector &InVector, double scale);
	
	float GetGroundFriction(float DeltaTime);
	
	/* 遗留空桩：设计意图对（这里就是 Source AngleVectors + 合成的位置），但从未实现、也无调用点。
	   路线 B 的实现落在 CalcWishVel() / CalcWishDirAndSpeed()（见上）。
	   本声明与空实现一并保留，未删除。 */
	void CalcVector(const FRotator &Angles, float Forward);
	
	public:
	virtual void DrawCameraDebugline(bool bDevMode);
};
