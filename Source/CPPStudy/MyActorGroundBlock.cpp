// Fill out your copyright notice in the Description page of Project Settings.


#include "MyActorGroundBlock.h"

#include "DiffUtils.h"
#include "AI/NavigationModifier.h"
#include "Components/StaticMeshComponent.h"
#include "Components/BoxComponent.h"

// Sets default values
AMyActorGroundBlock::AMyActorGroundBlock()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	
	BasicBox = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BasicBox"));
	RootComponent = BasicBox;
	
	//Set TriggerBox
	TriggerBox = CreateDefaultSubobject<UBoxComponent>("TriggerBox");
	TriggerBox->SetupAttachment(RootComponent);
	TriggerBox->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	
}

// Called when the game starts or when spawned
void AMyActorGroundBlock::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AMyActorGroundBlock::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

