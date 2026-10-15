# CPPStudy 测量场导入

在 Content Browser 中选 /Game/Code/Mesh/KZ_Measurement，然后 Import 两个 FBX。裸 FBX 放在 Content 目录不等于已经生成 UE 资源。

## 实体 SM_KZ_Measurement.fbx

- Static Mesh，Import Uniform Scale = 1。
- Import Collision = On（如果界面显示该项），Auto Generate Collision = Off。
- 文件内已附带 57 个 UCX_SM_KZ_Measurement_### 凸碰撞体，覆盖各平台与墙面，不跨越 gap。
- One Convex Hull per UCX = On（如果显示）；使用导入的简单碰撞，不要对整个场地生成单个凸包。
- 导入材质可开；没有贴图需要导入。

## 标牌和刻度 SM_KZ_Measurement_Marks.fbx

- Static Mesh，Import Uniform Scale = 1。
- Auto Generate Collision = Off。
- 导入后打开此 Static Mesh，设置 Collision Presets = NoCollision；也可在关卡里的组件上设为 NoCollision。
- 94 个 Blender FONT 标牌已在导出副本中转换为 mesh，源 .blend 保持文字可编辑。

## 放入地图

分别放两个资源，将两者 Location / Rotation 都设 0,0,0，Scale 都设 1,1,1，然后一起移动（或组起来）。
两文件采用相同世界原点，不能分别用拖拽的随机落点定位。
用标定区的 100UU 立方体核对：在 UE 中应为 100 x 100 x 100 cm。不要先手动乘 100。
打开实体 Static Mesh 的 Simple Collision 显示，核对碰撞体数量与 gap 内没有绿色碰撞面。
若特定 5.7 导入器没有识别 UCX，可仅对实体设置 Use Complex Collision As Simple 作为静态角色移动测试的备选；标牌仍用 NoCollision。这不能用于场地自身的物理模拟。

这不是 .umap，不会自动生成 PlayerStart、速度 HUD 或掉落重生。当前 gap 仅使用 457.2UU 参考基准，并非用户实测 Dmax。
