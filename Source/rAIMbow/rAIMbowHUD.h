// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once 

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "rAIMbowHUD.generated.h"

UCLASS()
class ArAIMbowHUD : public AHUD
{
	GENERATED_BODY()

public:
	ArAIMbowHUD();

	int Points;
	UPROPERTY() int HighScore;
	UPROPERTY() float SavedSensitivity;

	FString FlagType;
	UFont* PointsFont;
	UFont* FlagFont;

	float XL;
	float YL;

	bool GameActive;

	FTimerHandle Timer;
	UFUNCTION() void TimerEnd();

	/** Primary draw call for the HUD */
	virtual void DrawHUD() override;

	virtual void AddPoints();
	USoundBase* SuccessSound;
	virtual void SubtractPoints();
	USoundBase* FailureSound;

private:
	/** Crosshair asset pointer */
	class UTexture2D* CrosshairTex;

};

