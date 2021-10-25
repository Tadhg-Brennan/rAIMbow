// Intellectual Property of Tadhg Brennan

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "rAIMbowSave.generated.h"

/**
 * 
 */
UCLASS()
class RAIMBOW_API UrAIMbowSave : public USaveGame
{
	GENERATED_BODY()
	
public:
	UPROPERTY() int StandardHighScore;
	UPROPERTY() int CumulativeHighScore;

	// The controller sensitivity, set by slider in pause menu
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Controller)
	float SavedSensitivity;
};
