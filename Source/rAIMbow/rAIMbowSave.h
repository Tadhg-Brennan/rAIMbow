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
	UPROPERTY() int HighScore;
};
