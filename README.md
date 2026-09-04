# UnrealStudyProject

> ### Achieve Source Engine's Bhop (Strafe) in Unreal Engine 5
> 个人引擎研究项目：把 Source Engine（Source SDK 2013，`gamemovement.cpp`）的移动物理移植进 UE5 的 `CharacterMovementComponent` 层，复现经典 Bhop / Air Strafe 手感。

---

## 项目简介

作为 4000+ 小时 CS 玩家，我希望证明一件事：**Bhop / Strafe 的手感背后有一套精确的物理模型，而不是"引擎的玄学"**。

本项目不满足于"抄公式"，而是沿着 Source 的完整物理管线走了一遍：

```
玩家输入 → wishvel / wishdir → accelerate → friction → velocity → 位置
```

然后在 UE5 的 CMC（CharacterMovementComponent）架构中复刻这套规则。移植原则：

> **保留 UE 的移动框架（输入管线、碰撞、移动模式），替换其中的"速度更新规则"。**

---

## 技术原理

### 1. 输入映射：Source 的 `fmove/smove` 如何对应 UE 的 `Acceleration`

这是移植中最容易搞错的点，以下结论对照 `source-sdk-2013` 逐行核对得出：

- `Acceleration.GetSafeNormal2D()` **≡ Source 的 `wishdir`**
- `Acceleration.Size() / MaxAcceleration` **≡ Source 的 `wishspeed / m_maxspeed`**
- Source 在移动组件内部做 `AngleVectors`；UE 已把这一步前移到输入层（`AddMovementInput`），所以 CMC 里看不到 `m_flForwardMove` 原始标量——**它没有消失，只是被"旋转+合成"成了世界空间向量**

### 2. 为什么必须重写 `CalcVelocity`（而不是叠加）

UE5 默认地面逻辑会**直接杀死 Bhop 速度**：

1. 默认有效摩擦 = `GroundFriction(8.0) × BrakingFrictionFactor(2.0) = 16.0`
2. 速度被钳制在 `MaxWalkSpeed (600)`
3. 方向修正摩擦会把速度向加速度方向做 lerp，改变运动方向

所以要复刻 Bhop，必须自己实现地面分支（地面不参考 Z 轴速度 + Source 式摩擦先于加速）。

### 3. Source 加速模型的还原

- **空气加速 `AirAccelerate`**：以 `wishdir/wishspeed` 为准，按 `accelspeed = wishspeed * dt * acceleration` 计算增速，并钳制 `accelspeed > addspeed → addspeed`，实现"空中变向不掉速"
- **分步重力**：`StartGravity / FinishGravity` 各施加一半重力（那个著名的 `0.5`），整个帧内重力逐渐作用，更符合跳跃手感
- **地面摩擦**：按 `stop speed / friction` 系数衰减，静止相对传送带不减速

---

## Features

- [x] 自定义 `USourceCharacterMovementComponent`：重写 `CalcVelocity`，拆分 `AirMove` / `WalkMove` / `ApplyFriction`
- [x] 空气加速（Bhop / Air Strafe 核心）与地面摩擦模型
- [x] 空中连跳（落地自动触发 + 输入事件绑定）
- [x] 检查点重生系统：碰撞盒重叠 → 实时更新重生点 → 高度阈值掉落检测 → 复活传送；未触碰检查点时回退 `PlayerStart` 出生点
- [x] 第一/第三人称双相机系统（SpringArm + CameraComponent 即时切换）
- [x] UMG 调试面板（`BindWidget` + `AddDynamic` 委托绑定）
- [x] Enhanced Input 体系（PlayerController 侧集中注册 IMC）
- [x] `DevMode` 统一调试开关：全部调试输出收敛到一个 `bDevMode`，正常游玩零打扰
- [ ] KZ 身法测量地图：刻度跑道 + gap 阵列 + 速度显示（进行中）
- [ ] `sv_*` 风格参数调优体系（进行中）

---

## 研究与笔记

- **博客**: [fearlessyuzuki.github.io](https://fearlessyuzuki.github.io)
- **核心研究笔记**：《Review the code from 2013 —— `gamemovement.cpp` 逐段剖析》，本人撰写，包含 `CheckParameters / PlayerMove / FullWalkMove / CategorizePosition` 的拆解，重点分析了分步重力（`StartGravity / FinishGravity` 各施加一半的 `0.5`）、视角同步归一化（`YAW ± 360` 处理）、ladder 特殊移动模式等机制。

---

## Citations（参考资源）

- [Valve Source SDK 2013 — gamemovement.cpp](https://github.com/ValveSoftware/source-sdk-2013/blob/master/src/game/shared/gamemovement.cpp)
- [Bilibili Tutorial (Chinese)](https://www.bilibili.com/video/BV1BsNheREEB/)
    - [YouTube (English Original)](https://youtu.be/gRqoXy-0d84)
- [Reddit — The Code Behind Quake's Movement Tricks Explained](https://www.reddit.com/r/programming/comments/kui5b8/the_code_behind_quakes_movement_tricks_explained/)
- [Wikipedia — Dot Product (点积)](https://zh.wikipedia.org/wiki/%E7%82%B9%E7%A7%AF)
- [Steam Workshop — Bhop Guide](https://steamcommunity.com/sharedfiles/filedetails/?id=184184420)

---

## 开发学习记录

一些过程中的经验与踩坑：

- **Live Coding 的坑**：Live Coding 热更新不会覆盖所有改动，涉及头文件/新增类时必须完整重编译，否则会出现"代码改了但行为不变"
- **蓝图协作**：C++ 类与蓝图紧密耦合，GameMode / Controller / 输入映射等必须在蓝图/Project Settings 侧配对配置
- **UE 官方文档**：难度不在 C++ 而在于需要先理解引擎概念（pawn/controller/component 生命周期），否则看起来非常吃力
- **调试规范**：所有调试输出（屏幕消息、DrawDebugLine）收敛到 `DevMode` 统一开关，避免每帧刷屏
