// Copyright Epic Games, Inc. All Rights Reserved.

#include "rAIMbowGameMode.h"
#include "rAIMbowHUD.h"
#include "rAIMbowCharacter.h"
#include "UObject/ConstructorHelpers.h"

ArAIMbowGameMode::ArAIMbowGameMode()
	: Super()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnClassFinder(TEXT("/Game/FirstPersonCPP/Blueprints/FirstPersonCharacter"));
	DefaultPawnClass = PlayerPawnClassFinder.Class;

	// use our custom HUD class
	HUDClass = ArAIMbowHUD::StaticClass();
}
