// Intellectual Property of Tadhg Brennan


#include "rAIMbowTarget.h"
#include "rAIMbowHUD.h"
#include "Particles/ParticleSystemComponent.h"
#include "UObject/ConstructorHelpers.h"
#include "Runtime/Engine/Classes/Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"


// Sets default values
ArAIMbowTarget::ArAIMbowTarget()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	//Create pole for target to sit on
	TargetPole = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("TargetPole"));
	static ConstructorHelpers::FObjectFinder<UStaticMesh> TargetPoleAsset(TEXT("/Game/StarterContent/Props/SM_PillarFrame"));
	TargetPole->SetStaticMesh(TargetPoleAsset.Object);
	RootComponent = TargetPole;

	// Create box to display flag
	TargetBox = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("TargetBox"));
	static ConstructorHelpers::FObjectFinder<UStaticMesh> TargetBoxAsset(TEXT("/Game/rAIMbowContent/Objects/TargetBox"));
	TargetBox->SetStaticMesh(TargetBoxAsset.Object);
	TargetBox->SetupAttachment(TargetPole);
	TargetBox->SetRelativeLocation(FVector(-10.0f, 0.0f, 300.0f));
	TargetBox->SetRelativeRotation(FRotator(0.0f, 0.0f, 0.0f));

	// Create smaller hitbox for the target
	TargetCentralHitBox = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("TargetCentralHitBox"));
	static ConstructorHelpers::FObjectFinder<UStaticMesh> TargetCentralHitBoxAsset(TEXT("/Game/rAIMbowContent/Objects/TargetCentralHitBox"));
	TargetCentralHitBox->SetStaticMesh(TargetCentralHitBoxAsset.Object);
	TargetCentralHitBox->SetupAttachment(TargetPole);
	//TargetCentralHitBox->SetRelativeLocation(FVector(-15.0f, 0.0f, 325.0f));
	TargetCentralHitBox->SetRelativeLocation(FVector(10.0f, 0.0f, 420.0f));
	TargetCentralHitBox->SetRelativeRotation(FRotator(0.0f, 0.0f, 0.0f));

	//Store animation to play on successful target hit
	static ConstructorHelpers::FObjectFinder<UParticleSystem> HeartAnimationAsset(TEXT("/Game/rAIMbowContent/HeartAnimation/HeartParticles"));
	HeartAnimation = HeartAnimationAsset.Object;

	// Create an array to store all possible flags
	FlagArray = {"Asexual", "Bisexual", "GayMale", "Intersex", "Lesbian", "LGBTQ", "NonBinary", "Pansexual", "Transgender"};

	// Set all Flag Materials to be used later
	static ConstructorHelpers::FObjectFinder<UMaterialInterface> AsexualFlagAsset(TEXT("/Game/rAIMbowContent/PrideFlagMaterials/AsexualPrideFlagMat.AsexualPrideFlagMat"));
	AsexualFlagMaterial = AsexualFlagAsset.Object;
	static ConstructorHelpers::FObjectFinder<UMaterialInterface> BisexualFlagAsset(TEXT("/Game/rAIMbowContent/PrideFlagMaterials/BisexualPrideFlagMat.BisexualPrideFlagMat"));
	BisexualFlagMaterial = BisexualFlagAsset.Object;
	static ConstructorHelpers::FObjectFinder<UMaterialInterface> GayMaleFlagAsset(TEXT("/Game/rAIMbowContent/PrideFlagMaterials/GayMalePrideFlagMat.GayMalePrideFlagMat"));
	GayMaleFlagMaterial = GayMaleFlagAsset.Object;
	static ConstructorHelpers::FObjectFinder<UMaterialInterface> IntersexFlagAsset(TEXT("/Game/rAIMbowContent/PrideFlagMaterials/IntersexPrideFlagMat.IntersexPrideFlagMat"));
	IntersexFlagMaterial = IntersexFlagAsset.Object;
	static ConstructorHelpers::FObjectFinder<UMaterialInterface> LesbianFlagAsset(TEXT("/Game/rAIMbowContent/PrideFlagMaterials/LesbianPrideFlagMat.LesbianPrideFlagMat"));
	LesbianFlagMaterial = LesbianFlagAsset.Object;
	static ConstructorHelpers::FObjectFinder<UMaterialInterface> LGBTQFlagAsset(TEXT("/Game/rAIMbowContent/PrideFlagMaterials/LGBTQPrideFlagMat.LGBTQPrideFlagMat"));
	LGBTQFlagMaterial = LGBTQFlagAsset.Object;
	static ConstructorHelpers::FObjectFinder<UMaterialInterface> NonBinaryFlagAsset(TEXT("/Game/rAIMbowContent/PrideFlagMaterials/NonBinaryPrideFlagMat.NonBinaryPrideFlagMat"));
	NonBinaryFlagMaterial = NonBinaryFlagAsset.Object;
	static ConstructorHelpers::FObjectFinder<UMaterialInterface> PansexualFlagAsset(TEXT("/Game/rAIMbowContent/PrideFlagMaterials/PansexualPrideFlagMat.PansexualPrideFlagMat"));
	PansexualFlagMaterial = PansexualFlagAsset.Object;
	static ConstructorHelpers::FObjectFinder<UMaterialInterface> TransgenderFlagAsset(TEXT("/Game/rAIMbowContent/PrideFlagMaterials/TransgenderPrideFlagMat.TransgenderPrideFlagMat"));
	TransgenderFlagMaterial = TransgenderFlagAsset.Object;
	
	// Initialise all flags as LGBTQ
	TargetBox->SetMaterial(0, LGBTQFlagMaterial);
	FlagType = "LGBTQ";
}

// Called when the game starts or when spawned
void ArAIMbowTarget::BeginPlay()
{
	Super::BeginPlay();
	

}

// Called every frame
void ArAIMbowTarget::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (Falling)
	{
		DropTarget(DeltaTime);
	}

	if (Rising)
	{
		RaiseTarget(DeltaTime);
	}
}

void ArAIMbowTarget::TargetHit()
{
	//Emit hearts from the target
	UGameplayStatics* SpawnEmitter;
	SpawnEmitter->SpawnEmitterAtLocation(GetWorld(), HeartAnimation, GetActorLocation() + FVector(0.0f, 0.0f, 400.0f),GetActorRotation());
	//Drop all targets
	TArray<AActor*> FoundTargets;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), ArAIMbowTarget::StaticClass(), FoundTargets);
	for (int i = 0; i < FoundTargets.Num(); i++) {
		ArAIMbowTarget* ThisTarget = Cast<ArAIMbowTarget>(FoundTargets[i]);
		// Allow targets to be hit while rising
		if (ThisTarget->Rising) {
			ThisTarget->Rising = false;
		}
		// Capture index, important for ensuring all flags show up once each
		ThisTarget->Index = i;
		ThisTarget->Falling = true;
	}
}

//Falls until target is flush with ground, then changes the flags
void ArAIMbowTarget::DropTarget(float DeltaTime) {
	TargetRotation = TargetPole->GetRelativeRotation().Pitch;
	if (Falling) {
		FRotator Rotation = FRotator(-DeltaTime * 200, 0.0f, 0.0f);
		TargetPole->AddRelativeRotation(FQuat(Rotation), false, 0, ETeleportType::None);
	}
	if (FMath::IsNearlyEqual(TargetRotation, -90.0f, 2.0f)) {
		Falling = false;
		//Change HUD Flag as targets hit the ground
		ArAIMbowHUD* HUD = Cast<ArAIMbowHUD>(GetWorld()->GetFirstPlayerController()->GetHUD());
		HUD->FlagType = FlagArray[FMath::RandRange(0, FlagArray.Num()-1)];
		FlagChange(Index);
	}
}

// Rises until roughly vertical again
void ArAIMbowTarget::RaiseTarget(float DeltaTime) {
	TargetRotation = TargetPole->GetRelativeRotation().Pitch;
	if (Rising) {
		FRotator Rotation = FRotator(DeltaTime * 200, 0.0f, 0.0f);
		TargetPole->AddRelativeRotation(FQuat(Rotation), false, 0, ETeleportType::None);
	}
	if (FMath::IsNearlyEqual(TargetRotation, 0.0f, 2.0f)) {
		Rising = false;
	}
}

// Change all flags, relocate them and then raise the targets (Uses given index to ensure all flags show up just once)
void ArAIMbowTarget::FlagChange(int RandomisedIndex) {
	FlagType = FlagArray[RandomisedIndex];
	if (FlagType == "Asexual") {
		TargetBox->SetMaterial(0, AsexualFlagMaterial);
	}
	else if (FlagType == "Bisexual") {
		TargetBox->SetMaterial(0, BisexualFlagMaterial);
	}
	else if (FlagType == "GayMale") {
		TargetBox->SetMaterial(0, GayMaleFlagMaterial);
	}
	else if (FlagType == "Intersex") {
		TargetBox->SetMaterial(0, IntersexFlagMaterial);
	}
	else if (FlagType == "Lesbian") {
		TargetBox->SetMaterial(0, LesbianFlagMaterial);
	}
	else if (FlagType == "LGBTQ") {
		TargetBox->SetMaterial(0, LGBTQFlagMaterial);
	}
	else if (FlagType == "NonBinary") {
		TargetBox->SetMaterial(0, NonBinaryFlagMaterial);
	}
	else if (FlagType == "Pansexual") {
		TargetBox->SetMaterial(0, PansexualFlagMaterial);
	}
	else if (FlagType == "Transgender") {
		TargetBox->SetMaterial(0, TransgenderFlagMaterial);
	}

	//Randomise target location
	float NewX = FMath::FRandRange(-1150.0f, 1550.0f);
	float NewY = FMath::FRandRange(-1850.0f, 1850.0f);
	TargetPole->SetWorldLocation(FVector(NewX, NewY, 150.0f));

	Rising = true;
}