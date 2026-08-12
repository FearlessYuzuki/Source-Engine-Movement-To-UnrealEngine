# UnrealStudyProject

> ### Achieve Source Engine's Bhop (Strafe) in Unreal Engine 

---

## Citations

- [Bilibili Tutorial (Chinese)](https://www.bilibili.com/video/BV1BsNheREEB/?spm_id_from=333.337.search-card.all.click&vd_source=2b0cc67f58126502375e3372458007f9)
    - [YouTube (English Original)](https://youtu.be/gRqoXy-0d84?si=r_Vu7vnjUx1-GGyv)
- [Valve Source SDK 2013 — gamemovement.cpp](https://github.com/ValveSoftware/source-sdk-2013/blob/master/src/game/shared/gamemovement.cpp)
- [Reddit — The Code Behind Quake's Movement Tricks Explained](https://www.reddit.com/r/programming/comments/kui5b8/the_code_behind_quakes_movement_tricks_explained/)
- [Wikipedia — Dot Product (点积)](https://zh.wikipedia.org/wiki/%E7%82%B9%E7%A7%AF)
- [Steam Workshop — Bhop Guide](https://steamcommunity.com/sharedfiles/filedetails/?id=184184420)

---

## 开发学习过程（一些收获）

- 在 UE 中蓝图是和 C++ 紧密贴合的，而且 UE 本身的 C++ 类创建的时候就很明确了功能，要看清楚。

- UE 的 **Live Coding** 功能是热更新，但是不会长久保存，你需要在编译器里手动编译一次，C++ 类才会在 `CppClass` 文件夹可见。

- 记得要在蓝图里设置相关游戏内容——GameMode、Controller 什么乱七八糟的。

- 官方的 UE 文档很好用，但是写的有一定门槛，不是 C++ 门槛，是对引擎本身需要有一定的理解，不然看起来非常吃力。

## CS的规则 （Counter-Strikes Bhop Rules）
### 这里整体的规则并不是解包或者怎样得来的 全部是基于游戏的观察和游戏经验（4000+h cser） 以及已经开放的 “Source SDK 2013” 总结得来

- 当出发完美连跳时，地速是不会受到影响的，当然是在原有的控制台指令sv_enablebunnyhop true 条件下是这样的 官方的做法是低速不会高于300 高于300就会限制速度 或者说掉速 当上述指令开启时 低速会解除限制 此时影响速度的只有airwishspeed(默认为30) 以及airaccleration(默认为12) 

- 另外在起源引擎中 如果角色当前速度不满足角色最大速度时 键入W在空中保持完美连跳其实无用 因为w的键入并不会有实质性的速度增加（原理未知）

- 同样拥有惯性，角色移动并不是一秒变为0

- 到达最大速度时 在竞技模式下 如果下一次的速度(这里指当前速度+期望速度（或者说是加速度）)总量超过了300 立刻减速