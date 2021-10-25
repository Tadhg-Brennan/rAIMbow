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
	static ConstructorHelpers::FObjectFinder<UTexture2D> CrosshairTexObj(TEXT("/Game/FirstPerson/Textures/HeartCrosshair"));
	CrosshairTex = CrosshairTexObj.Object;

	// Load any existing high score. If there are no saves, set high scores to 0
	if (UGameplayStatics::DoesSaveGameExist("rAIMbowSave", 1)) {
		UrAIMbowSave* LoadedSave = Cast<UrAIMbowSave>(UGameplayStatics::CreateSaveGameObject(UrAIMbowSave::StaticClass()));
		LoadedSave = Cast<UrAIMbowSave>(UGameplayStatics::LoadGameFromSlot("rAIMbowSave", 1));
		StandardHighScore = LoadedSave->StandardHighScore;
		CumulativeHighScore = LoadedSave->CumulativeHighScore;
	}
	else {
		StandardHighScore = 0;
		CumulativeHighScore = 0;
	}

	// Initialise with the game inactive, game type as standard, 0 points, and LGBTQ as the HUD flag
	GameActive = false;
	Points = 0;
	FlagType = "LGBTQ";
	GameType = "Standard";

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
	FCanvasTextItem HighScoreText(FVector2D::ZeroVector, FText::FromString(*FString("High Score: " + FString::FromInt(StandardHighScore))), PointsFont, FLinearColor::White);
	if (GameType == "Standard") {
		HighScoreText.Text = FText::FromString(*FString("High Score: " + FString::FromInt(StandardHighScore)));
	}
	else if (GameType == "Cumulative") {
		HighScoreText.Text = FText::FromString(*FString("High Score: " + FString::FromInt(CumulativeHighScore)));
	}
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
		if (GameType == "Standard") {
			GetWorld()->GetTimerManager().SetTimer(Timer, this, &ArAIMbowHUD::TimerEnd, 60.0f, false);
		}
		else if (GameType == "Cumulative") {
			GetWorld()->GetTimerManager().SetTimer(Timer, this, &ArAIMbowHUD::TimerEnd, 5.0f, false);
		}
		GameActive = true;
	}
	else {
		if (GameType == "Cumulative") {
			GetWorld()->GetTimerManager().SetTimer(Timer, this, &ArAIMbowHUD::TimerEnd, 2.0f + GetWorld()->GetTimerManager().GetTimerRemaining(Timer), false);
		}
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

	// Load sensitivity in case it was set during a game, otherwise set defaults
	if (UGameplayStatics::DoesSaveGameExist("rAIMbowSave", 1)) {
		UrAIMbowSave* LoadedSave = Cast<UrAIMbowSave>(UGameplayStatics::CreateSaveGameObject(UrAIMbowSave::StaticClass()));
		LoadedSave = Cast<UrAIMbowSave>(UGameplayStatics::LoadGameFromSlot("rAIMbowSave", 1));
		SavedSensitivity = LoadedSave->SavedSensitivity;
	}
	else {
		StandardHighScore = 0;
		CumulativeHighScore = 0;
		SavedSensitivity = 0.45;
	}

	// Save if a new high score has been achieved
	if (Points > StandardHighScore && GameType == "Standard") {
		StandardHighScore = Points;
		UrAIMbowSave* Save = Cast<UrAIMbowSave>(UGameplayStatics::CreateSaveGameObject(UrAIMbowSave::StaticClass()));
		Save->StandardHighScore = StandardHighScore;
		Save->CumulativeHighScore = CumulativeHighScore;
		Save->SavedSensitivity = SavedSensitivity;
		UGameplayStatics::SaveGameToSlot(Save, "rAIMbowSave", 1);
	}
	else if (Points > CumulativeHighScore && GameType == "Cumulative") {
		CumulativeHighScore = Points;
		UrAIMbowSave* Save = Cast<UrAIMbowSave>(UGameplayStatics::CreateSaveGameObject(UrAIMbowSave::StaticClass()));
		Save->StandardHighScore = StandardHighScore;
		Save->CumulativeHighScore = CumulativeHighScore;
		Save->SavedSensitivity = SavedSensitivity;
		UGameplayStatics::SaveGameToSlot(Save, "rAIMbowSave", 1);
	}

	//Reset points and all targets
	Points = 0;
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
