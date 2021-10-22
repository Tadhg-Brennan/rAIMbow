// Intellectual Property of Tadhg Brennan

#pragma once

#include "CoreMinimal.h"
#include "Particles/ParticleSystemComponent.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "Components/BoxComponent.h"
#include "Math/UnrealMathUtility.h"
#include "rAIMbowTarget.generated.h"


UCLASS()
class RAIMBOW_API ArAIMbowTarget : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ArAIMbowTarget();

	FString FlagType;

	TArray<FString> FlagArray;
	int Index;

	UParticleSystem* HeartAnimation;

	// Initialise pride flag materials
	UMaterialInterface* AsexualFlagMaterial;
	UMaterialInterface* BisexualFlagMaterial;
	UMaterialInterface* GayMaleFlagMaterial;
	UMaterialInterface* IntersexFlagMaterial;
	UMaterialInterface* LesbianFlagMaterial;
	UMaterialInterface* LGBTQFlagMaterial;
	UMaterialInterface* NonBinaryFlagMaterial;
	UMaterialInterface* PansexualFlagMaterial;
	UMaterialInterface* TransgenderFlagMaterial;

	// Initialise components of the physical target object
	UStaticMeshComponent* TargetBox;
	UStaticMeshComponent* TargetPole;
	UStaticMeshComponent* TargetCentralHitBox;

	// Values used to govern the target movement
	bool Falling;
	bool Rising;
	float TargetRotation;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;
	virtual void DropTarget(float DeltaTime);
	virtual void RaiseTarget(float DeltaTime);
	virtual void TargetHit();
	virtual void FlagChange(int RandomisedIndex);
};
