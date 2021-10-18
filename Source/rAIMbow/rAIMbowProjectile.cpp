// Copyright Epic Games, Inc. All Rights Reserved.

#include "rAIMbowProjectile.h"
#include "rAIMbowTarget.h"
#include "rAIMbowHUD.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Components/SphereComponent.h"

ArAIMbowProjectile::ArAIMbowProjectile()
{
	// Use a sphere as a simple collision representation
	CollisionComp = CreateDefaultSubobject<USphereComponent>(TEXT("SphereComp"));
	CollisionComp->InitSphereRadius(0.2f);
	CollisionComp->BodyInstance.SetCollisionProfileName("Projectile");
	CollisionComp->OnComponentHit.AddDynamic(this, &ArAIMbowProjectile::OnHit);		// set up a notification for when this component hits something blocking

	// Players can't walk on it
	CollisionComp->SetWalkableSlopeOverride(FWalkableSlopeOverride(WalkableSlope_Unwalkable, 0.f));
	CollisionComp->CanCharacterStepUpOn = ECB_No;

	// Set as root component
	RootComponent = CollisionComp;

	// Use a ProjectileMovementComponent to govern this projectile's movement
	ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileComp"));
	ProjectileMovement->UpdatedComponent = CollisionComp;
	ProjectileMovement->InitialSpeed = 10000.f;
	ProjectileMovement->MaxSpeed = 10000.f;
	ProjectileMovement->bRotationFollowsVelocity = true;
	ProjectileMovement->bShouldBounce = false;
	ProjectileMovement->ProjectileGravityScale = 0;

	// Die after 10 seconds by default
	InitialLifeSpan = 10.0f;

	// Create the visible portion of the projectile
	ProjectileParticle = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ProjectileParticle"));
	static ConstructorHelpers::FObjectFinder<UStaticMesh> ProjectileParticleAsset(TEXT("/Game/rAIMbowContent/Objects/ProjectileObject"));
	ProjectileParticle->SetStaticMesh(ProjectileParticleAsset.Object);
	ProjectileParticle->SetupAttachment(CollisionComp);
	ProjectileParticle->SetRelativeLocation(FVector(100.0f, 0.0f, 0.0f));
	ProjectileParticle->SetRelativeRotation(FRotator(90.0f, 0.0f, 0.0f));
}

void ArAIMbowProjectile::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	if ((OtherActor != nullptr) && (OtherActor != this) && (OtherComp != nullptr))
	{
		// Add impulse if we hit a physics (This was mainly used in shot testing, left in in case of future developments)
		if (OtherComp->IsSimulatingPhysics()) {
			OtherComp->AddImpulseAtLocation(GetVelocity() * 100.0f, GetActorLocation());
		}
		if (OtherComp->GetName() == "TargetCentralHitBox") {
			ArAIMbowTarget* Target = Cast<ArAIMbowTarget>(OtherActor);
			ArAIMbowHUD* HUD = Cast<ArAIMbowHUD>(GetWorld()->GetFirstPlayerController()->GetHUD());
			if (Target->FlagType == HUD->FlagType) {
				//Add points if you hit the correct target
				HUD->AddPoints();
				//Update the targets
				Target->TargetHit();
			}
			else {
				//Subtract points if you hit the wrong target
				HUD->SubtractPoints();
			}
		}
		// Destroy particle regardless of what is hit
		Destroy();
	}
}