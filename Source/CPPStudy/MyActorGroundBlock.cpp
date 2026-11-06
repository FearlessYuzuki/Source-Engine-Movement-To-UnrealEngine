// Fill out your copyright notice in the Description page of Project Settings.


#include "MyActorGroundBlock.h"
#include "Components/StaticMeshComponent.h"
#include "Components/BoxComponent.h"

class Enigne;

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
	TriggerBox->SetGenerateOverlapEvents(true);
	
	
}

// Called when the game starts or when spawned
void AMyActorGroundBlock::BeginPlay()
{
	Super::BeginPlay();
	if (TriggerBox)
	{
		GEngine->AddOnScreenDebugMessage(-1,10,FColor::Red,TEXT("Trigger Box Begin"));
	}
	TriggerBox->OnComponentBeginOverlap.AddDynamic(this,&AMyActorGroundBlock::DebugTriggered);
}

// Called every frame
void AMyActorGroundBlock::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AMyActorGroundBlock::DebugTriggered(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, 
	int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	GEngine->AddOnScreenDebugMessage(-1,1,FColor::Emerald,
		TEXT("Player is overlapping!!"));
	UE_LOG(LogTemp, Warning, TEXT("绑定成功！"));
}


