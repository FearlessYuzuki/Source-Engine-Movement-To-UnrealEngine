# UnrealStudyProject

> ### Achieve Source Engine's Bhop (Strafe) in Unreal Engine 5
> 个人引擎研究项目：把 Source Engine（Source SDK 2013，`gamemovement.cpp`）的移动物理移植进 UE5 的 `CharacterMovementComponent` 层，复现经典 Bhop / Air Strafe 手感。

---

## 项目简介

作为 4000+ 小时 CS 玩家，最近也在往游戏开发发展 我想实现的是**Bhop / Strafe 这一近乎是Source系引擎独占的身法魅力 是可以在UE里复刻出来的（也是对瓦罗兰特糟糕的移动的好奇）"**。

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
- [x] KZ 身法测量地图：刻度跑道 + gap 阵列 + 速度显示
- [ ] `sv_*` 风格参数调优体系（进行中）

---

## 研究与笔记

- **博客**: [fearlessyuzuki.github.io](https://fearlessyuzuki.github.io)(正在重构中 暂时停止更新)
- **核心研究笔记**：《Review the code from 2013 —— `gamemovement.cpp` 逐段剖析》，本人撰写(都是个人见解 AI含量很低 最多的AI是用来规则格式)，包含 `CheckParameters / PlayerMove / FullWalkMove / CategorizePosition` 的拆解，重点分析了分步重力（`StartGravity / FinishGravity` 各施加一半的 `0.5`）、视角同步归一化（`YAW ± 360` 处理）、ladder 特殊移动模式等机制。

---

## 后续开发

- **路线**: 近期受到来自2017年在Steam上的游戏 Refunct的启发 本项目的移动完全可以结合UE的优势也就是画面和光照来进行更进一步的开发
- ###### 渲染管线
- ###### 地面变化
- ###### 角色滑铲（下蹲）
等等...

---
## AI 使用注意！！

> ⚠️ **本项目包含 AI 工具辅助开发。**

### 开发环境

- **开发工具**：Pi Coding Agent
- **模型组合**：混合模型
  - `deepseek-flash`
  - `mimo-2.6-pro`
  - `claude opus 5.5`
  - `gpt-sol-6.1`

本人使用 **Pi Coding Agent**，搭配 `deepseek-flash`、`mimo-2.6-pro`、`claude opus 5.5`、`gpt-sol-6.1` 混合模型进行开发。

### 本人完成

- **CMC 重写逻辑**：核心重写逻辑由本人编写。
- **CMC Override**：由本人测试并实现。
- **项目整体设计与核心功能**：由本人完成。
- **角色摄像机人称切换**：由本人编写。
- **HUD 搭建**：由本人完成。

### AI 参与

**AI 辅助内容：**

- **LLM 使用范围**：仅用于 **Code Explanation**、**轻量开发**、**文本格式美化**。
- **代码注释**：AI 辅助编写、润色、补全。
- **灵感 / 思考 / 启发注释**：AI 辅助整理和记录。
- **少量非核心边角功能**：AI 辅助完成。

**AI 参与的功能性内容：**

- **重生点 / 检查点设置和调试**
- **Source / UE 地图测量场**：通过 **AI + Blender** 实现。

---

## Citations（参考资源）

- [Valve Source SDK 2013 — gamemovement.cpp](https://github.com/ValveSoftware/source-sdk-2013/blob/master/src/game/shared/gamemovement.cpp)
- [Bilibili Tutorial (Chinese)](https://www.bilibili.com/video/BV1BsNheREEB/)
    - [YouTube (English Original)](https://youtu.be/gRqoXy-0d84)
- [Reddit — The Code Behind Quake's Movement Tricks Explained](https://www.reddit.com/r/programming/comments/kui5b8/the_code_behind_quakes_movement_tricks_explained/)
- [Wikipedia — Dot Product (点积)](https://zh.wikipedia.org/wiki/%E7%82%B9%E7%A7%AF)
- [Steam Workshop — Bhop Guide](https://steamcommunity.com/sharedfiles/filedetails/?id=184184420)

---



