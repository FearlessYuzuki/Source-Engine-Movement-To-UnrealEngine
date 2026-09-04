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

void AMyRespawnActor::OnTriggerBoxBeginOverlap(UPrimitiveComponent* OverlappedComponent,
	AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
	bool bFromSweep, const FHitResult& SweepResult)
{
	if (APlayerCharacter* Player = Cast<APlayerCharacter>(OtherActor))
	{
		Player->SetSpawnPoint(this);
	}
}
