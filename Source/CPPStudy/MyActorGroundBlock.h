// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/BoxComponent.h"
#include "MyActorGroundBlock.generated.h"

UCLASS()
class CPPSTUDY_API AMyActorGroundBlock : public AActor
{
	GENERATED_BODY()
		
public:
	UPROPERTY(VisibleAnywhere,BlueprintReadWrite,Category="SceneComponent",meta = (AllowPrivateAccess = true))
	TObjectPtr<UStaticMeshComponent> BasicBox;
	
	UPROPERTY(VisibleAnywhere,BlueprintReadWrite,Category="SceneComponent",meta = (AllowPrivateAccess = true))
	TObjectPtr<UBoxComponent> TriggerBox;
	
	UPROPERTY(VisibleAnywhere,BlueprintReadWrite,Category="SceneComponent",meta = (AllowPrivateAccess = true))
	TObjectPtr<UStaticMeshComponent> Cubical;
	
	UPROPERTY(EditAnywhere,BlueprintReadWrite,Category="BlockMaterial",meta = (AllowPrivateAccess = true))
	TObjectPtr<UMaterialInterface> BasicMaterialInterface;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resources")
	TObjectPtr<UStaticMesh> BaseMesh;
	
public:	
	// Sets default values for this actor's properties
	AMyActorGroundBlock();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	
public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;
	
	UFUNCTION()
	void DebugTriggered(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
	
	UFUNCTION()
	void ChangeBlockTexture(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, 
	int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
	
	UPROPERTY(BlueprintReadWrite,EditAnywhere,Category="Overlapped Material Changes settings")
	UMaterialInterface* OverlappedMaterial;
};

