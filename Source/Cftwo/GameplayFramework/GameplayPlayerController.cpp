// Fill out your copyright notice in the Description page of Project Settings.


#include "GameplayPlayerController.h"
#include "GameplayGameMode.h"
#include "GameplayHUD.h"
#include "Kismet/GameplayStatics.h"
#include "Characters/GameplayCharacter.h"

void AGameplayPlayerController::Server_AskToRespawn_Implementation()
{
	AGameplayGameMode* GMRef = Cast<AGameplayGameMode>(UGameplayStatics::GetGameMode(GetWorld()));
	GMRef->SpawnPlayerCharacter(this);
}

void AGameplayPlayerController::Client_AskToPunch_Implementation()
{
	if (!Cast<AGameplayCharacter>(GetPawn()))
		return;
	Cast<AGameplayCharacter>(GetPawn())->OnHit();
}

void AGameplayPlayerController::BeginPlay()
{
	Super::BeginPlay();

	SetShowMouseCursor(true);
}

void AGameplayPlayerController::Client_SetPawnInHUD_Implementation()
{
	if (!GetHUD())
		return;

	Cast<AGameplayHUD>(GetHUD())->CharacterRef = Cast<AGameplayCharacter>(GetPawn());
}

void AGameplayPlayerController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	Client_SetPawnInHUD();
}
