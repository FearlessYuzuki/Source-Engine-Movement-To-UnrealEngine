// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/BoxComponent.h"s
#include "MyActorGroundBlock.generated.h"

UCLASS()
class CPPSTUDY_API AMyActorGroundBlock : public AActor
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere,BlueprintReadWrite,Category="SceneComponent",meta = (AllowPrivateAccess = true))
	TObjectPtr<UStaticMeshComponent> BasicBox;
	
	UPROPERTY(EditAnywhere,BlueprintReadWrite,Category="SceneComponent",meta = (AllowPrivateAccess = true))
	TObjectPtr<UBoxComponent> TriggerBox;
	
public:	
	// Sets default values for this actor's properties
	AMyActorGroundBlock();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	
public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;
	
};
