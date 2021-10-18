// Copyright Epic Games, Inc. All Rights Reserved.

#include "rAIMbowHUD.h"
#include "rAIMbowTarget.h"
#include "rAIMbowSave.h"
#include "Engine/Canvas.h"
#include "Engine/Texture2D.h"
#include "TextureResource.h"
#include "CanvasItem.h"
#include "Kismet/GameplayStatics.h"
#include "Templates/Casts.h"
#include "UObject/ConstructorHelpers.h"

ArAIMbowHUD::ArAIMbowHUD()
{
	// Set the crosshair texture
	static ConstructorHelpers::FObjectFinder<UTexture2D> CrosshairTexObj(TEXT("/Game/FirstPerson/Textures/FirstPersonCrosshair"));
	CrosshairTex = CrosshairTexObj.Object;

	// Load any existing high score. If there are no saves, set high score to 0
	if (UGameplayStatics::DoesSaveGameExist("rAIMbowSave", 1)) {
		UrAIMbowSave* LoadedSave = Cast<UrAIMbowSave>(UGameplayStatics::CreateSaveGameObject(UrAIMbowSave::StaticClass()));
		LoadedSave = Cast<UrAIMbowSave>(UGameplayStatics::LoadGameFromSlot("rAIMbowSave", 1));
		HighScore = LoadedSave->HighScore;
	}
	else {
		HighScore = 0;
	}

	// Initialise with the game inactive, 0 points, and LGBTQ as the HUD flag
	GameActive = false;
	Points = 0;
	FlagType = "LGBTQ";

	// Set fonts and audio to be used
	ConstructorHelpers::FObjectFinder<UFont> PointsFontObject(TEXT("Font'/Game/rAIMbowContent/Font/PixelFont.PixelFont'"));
	PointsFont = PointsFontObject.Object;
	ConstructorHelpers::FObjectFinder<UFont> FlagFontObject(TEXT("Font'/Game/rAIMbowContent/Font/LoveSongFont.LoveSongFont'"));
	FlagFont = FlagFontObject.Object;
	ConstructorHelpers::FObjectFinder<USoundBase> SuccessAudio(TEXT("/Game/rAIMbowContent/Audio/FabulousAudio"));
	SuccessSound = SuccessAudio.Object;
	ConstructorHelpers::FObjectFinder<USoundBase> FailureAudio(TEXT("/Game/rAIMbowContent/Audio/ScrewYouAudio"));
	FailureSound = FailureAudio.Object;
}


void ArAIMbowHUD::DrawHUD()
{
	Super::DrawHUD();

	// Draw very simple crosshair

	// find center of the Canvas
	const FVector2D Center(Canvas->ClipX * 0.5f, Canvas->ClipY * 0.5f);

	// offset by half the texture's dimensions so that the center of the texture aligns with the center of the Canvas
	const FVector2D CrosshairDrawPosition( (Center.X - 8.0f),
										   (Center.Y - 8.0f));

	// draw the crosshair
	FCanvasTileItem TileItem( CrosshairDrawPosition, CrosshairTex->Resource, FLinearColor::White);
	TileItem.BlendMode = SE_BLEND_Translucent;
	Canvas->DrawItem( TileItem );

	// Create an X-centred Flag display 
	FCanvasTextItem FlagText(FVector2D::ZeroVector, FText::FromString(FlagType), FlagFont, FLinearColor::White);
	if (FlagType == "NonBinary") {
		FlagText.Text = FText::FromString("Non-Binary");
	}
	else if (FlagType == "GayMale") {
		FlagText.Text = FText::FromString("Gay Male");
	}
	else {
		FlagText.Text = FText::FromString(*FlagType);
	}
	Canvas->StrLen(FlagFont, FlagText.Text.ToString(), XL, YL);
	Canvas->DrawItem(FlagText, Center.X -XL/2, 10.0f);
	
	// Create a Points display 
	FCanvasTextItem PointsText(FVector2D::ZeroVector, FText::FromString(*FString::FromInt(Points)), PointsFont, FLinearColor::White);
	PointsText.Text = FText::FromString(*FString("Points: " + FString::FromInt(Points)));
	Canvas->DrawItem(PointsText, 10.0f, 10.0f);

	// Create a High Score display 
	FCanvasTextItem HighScoreText(FVector2D::ZeroVector, FText::FromString(*FString("High Score: " + FString::FromInt(HighScore))), PointsFont, FLinearColor::White);
	HighScoreText.Text = FText::FromString(*FString("High Score: " + FString::FromInt(HighScore)));
	Canvas->DrawItem(HighScoreText, 10.0f, 50.0f);

	//Create a Time display
	if (GameActive) {
		FCanvasTextItem TimerText(FVector2D::ZeroVector, FText::FromString(*FString::SanitizeFloat(GetWorld()->GetTimerManager().GetTimerRemaining(Timer)).Left(5)), PointsFont, FLinearColor::White);
		TimerText.Text = FText::FromString(*FString::SanitizeFloat(GetWorld()->GetTimerManager().GetTimerRemaining(Timer)).Left(5));
		Canvas->DrawItem(TimerText, 2*Center.X - 90.0f, 10.0f);
	}
}

// Add points and play success sound for hitting the correct targets. First hit begins the timer and game.
void ArAIMbowHUD::AddPoints()
{
	UGameplayStatics::PlaySoundAtLocation(this, SuccessSound, GetActorLocation());
	if (!GameActive) {
		GetWorld()->GetTimerManager().SetTimer(Timer, this, &ArAIMbowHUD::TimerEnd, 60.0f, false);
		GameActive = true;
	}
	else {
		Points += 100;
	}
}

// Subtract points and play failure sound for hitting the incorrect targets
void ArAIMbowHUD::SubtractPoints()
{
	Points -= 50;
	UGameplayStatics::PlaySoundAtLocation(this, FailureSound, GetActorLocation());
}

// On timer end, reset points and all targets. Save any new high scores.
void ArAIMbowHUD::TimerEnd() {
	GameActive = false;

	// Save if a new high score has been achieved
	if (Points > HighScore) {
		HighScore = Points;
		UrAIMbowSave* Save = Cast<UrAIMbowSave>(UGameplayStatics::CreateSaveGameObject(UrAIMbowSave::StaticClass()));
		Save->HighScore = HighScore;
		UGameplayStatics::SaveGameToSlot(Save, "rAIMbowSave", 1);
	}

	Points = 0;

	//Reset all targets
	TArray<AActor*> FoundTargets;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), ArAIMbowTarget::StaticClass(), FoundTargets);
	for (int i = 0; i < FoundTargets.Num(); i++) {
		ArAIMbowTarget* ThisTarget = Cast<ArAIMbowTarget>(FoundTargets[i]);
		ThisTarget->Rising = false;
		ThisTarget->Falling = false;
		ThisTarget->TargetPole->SetWorldRotation(FRotator(0.0f, 0.0f, 0.0f));
		if (i <= 4) {
			ThisTarget->TargetPole->SetWorldLocation(FVector(-1000 + i * 500.0f, -1000 + i * 250.0f, 150.0f));
		}
		else {
			ThisTarget->TargetPole->SetWorldLocation(FVector(1000 - (i-4) * 500.0f, -1000 + i * 250.0f, 150.0f));
		}
		ThisTarget->FlagType = "LGBTQ";
		ThisTarget->TargetBox->SetMaterial(0, ThisTarget->LGBTQFlagMaterial);
		FlagType = "LGBTQ";
	}
}
