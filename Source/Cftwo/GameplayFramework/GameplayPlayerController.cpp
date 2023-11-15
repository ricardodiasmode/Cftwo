// Fill out your copyright notice in the Description page of Project Settings.


#include "GameplayPlayerController.h"
#include "GameplayGameMode.h"
#include "Kismet/GameplayStatics.h"
#include "Characters/GameplayCharacter.h"

void AGameplayPlayerController::Server_AskToRespawn_Implementation()
{
	AGameplayGameMode* GMRef = Cast<AGameplayGameMode>(UGameplayStatics::GetGameMode(GetWorld()));
	GMRef->SpawnPlayerCharacter(this);
}

void AGameplayPlayerController::Client_AskToPunch_Implementation()
{
	Cast<AGameplayCharacter>(GetPawn())->OnHit();
}

void AGameplayPlayerController::BeginPlay()
{
	Super::BeginPlay();

	SetShowMouseCursor(true);
	SetInputMode(FInputModeUIOnly());
}
