// Fill out your copyright notice in the Description page of Project Settings.

#include "MyRespawnActor.h"
#include "PlayerCharacter.h"
#include "Engine/Engine.h"

// Sets default values
AMyRespawnActor::AMyRespawnActor()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	BaseBox = CreateDefaultSubobject<UStaticMeshComponent>(FName("BaseBox"));
	RootComponent = BaseBox;
	
	BaseBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	TriggerBox = CreateDefaultSubobject<UBoxComponent>(FName("TriggerBox"));
	TriggerBox->SetupAttachment(BaseBox);
	TriggerBox->SetCollisionProfileName(FName("Trigger"));
	TriggerBox->SetGenerateOverlapEvents(true);
	
	TriggerBox->OnComponentBeginOverlap.AddDynamic(this, &AMyRespawnActor::OnTriggerBoxBeginOverlap);
}

void AMyRespawnActor::BeginPlay()
{
	Super::BeginPlay();

	// ★ 根因修复：构造函数里的 AddDynamic 绑在 CDO 上，在 BP 实例上不会生效（实测验证）。
	// 这里在实例自己的组件上重新绑一次（先 Remove 防重复），让组件级委托真正工作。
	if (TriggerBox)
	{
		TriggerBox->OnComponentBeginOverlap.RemoveDynamic(this, &AMyRespawnActor::OnTriggerBoxBeginOverlap);
		TriggerBox->OnComponentBeginOverlap.AddDynamic(this, &AMyRespawnActor::OnTriggerBoxBeginOverlap);
		UE_LOG(LogTemp, Warning, TEXT("[CP] BeginPlay 重新绑定 TriggerBox 委托: %s (已绑数=%d)"),
			*TriggerBox->GetName(), TriggerBox->OnComponentBeginOverlap.IsBound() ? 1 : 0);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("[CP] BeginPlay 时 TriggerBox 为空！"));
	}
}

void AMyRespawnActor::OnTriggerBoxBeginOverlap(UPrimitiveComponent* OverlappedComponent,
	AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
	bool bFromSweep, const FHitResult& SweepResult)
{
	UE_LOG(LogTemp, Warning, TEXT("[CP] OnTriggerBoxBeginOverlap: %s (Class=%s)"),
		*GetNameSafe(OtherActor), OtherActor ? *OtherActor->GetClass()->GetName() : TEXT("null"));

	if (APlayerCharacter* Player = Cast<APlayerCharacter>(OtherActor))
	{
		Player->SetSpawnPoint(this);
		UE_LOG(LogTemp, Warning, TEXT("[CP] 重生点已设为 %s @ %s"), *GetName(), *GetActorLocation().ToString());
	}
}

// [诊断+加固] Actor 级入口：任何组件产生 overlap 都会到这里。
// 如果 PIE 里踩上圆盘后这里不打印，说明根本不是「代码没接上」，而是碰撞体压根没产生 overlap。
void AMyRespawnActor::NotifyActorBeginOverlap(AActor* OtherActor)
{
	Super::NotifyActorBeginOverlap(OtherActor);

	UE_LOG(LogTemp, Warning, TEXT("[CP] NotifyActorBeginOverlap: %s (Class=%s)"),
		*GetNameSafe(OtherActor), OtherActor ? *OtherActor->GetClass()->GetName() : TEXT("null"));

	// 到底哪个组件碰上的？圆盘还是触发盒？
	FString Overlapped;
	for (UActorComponent* Comp : GetComponents())
	{
		if (UPrimitiveComponent* Prim = Cast<UPrimitiveComponent>(Comp))
		{
			if (Prim->IsOverlappingActor(OtherActor))
			{
				Overlapped += Prim->GetName() + TEXT(" ");
			}
		}
	}
	UE_LOG(LogTemp, Warning, TEXT("[CP] 参与重叠的组件: [%s]"), *Overlapped);

	if (APlayerCharacter* Player = Cast<APlayerCharacter>(OtherActor))
	{
		Player->SetSpawnPoint(this);
		UE_LOG(LogTemp, Warning, TEXT("[CP] (actor级) 重生点已设为 %s"), *GetName());
	}
}
