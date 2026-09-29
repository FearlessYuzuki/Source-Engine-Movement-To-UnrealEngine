// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/Actor.h"
#include "MyRespawnActor.generated.h"

UCLASS()
class CPPSTUDY_API AMyRespawnActor : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AMyRespawnActor();

	UPROPERTY()
	TObjectPtr<UStaticMeshComponent> BaseBox;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	TObjectPtr<UBoxComponent> TriggerBox;

protected:
	// ★ 根因修复：构造函数里的 AddDynamic 在 BP 实例上不会生效（实测：实例上组件级委托从不触发，
	//   但 actor 级 NotifyActorBeginOverlap 能触发）。BeginPlay 里在实例自己的组件上重新绑一次，
	//   让原本设计意图的组件级路径也能工作。
	virtual void BeginPlay() override;

	UFUNCTION()
	void OnTriggerBoxBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	// [诊断+加固] Actor 级 overlap 通知。
	// 比组件级委托更不容易失效：只要本 Actor 的任何组件与对方产生 overlap 就会调到这里，
	// 不依赖 TriggerBox 那个 AddDynamic 委托还活着。两处可以并存，互不影响。
	// 定位完之后，日志可以降级或删除，这个 override 建议保留（当兜底）。
	virtual void NotifyActorBeginOverlap(AActor* OtherActor) override;
};
