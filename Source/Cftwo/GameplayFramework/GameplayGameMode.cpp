// Fill out your copyright notice in the Description page of Project Settings.

#include "GameplayGameMode.h"

#include "DefaultGameInstance.h"
#include "GameplayPlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerStart.h"
#include "Characters/GameplayCharacter.h"
#include "../Utils/GeneralFunctionLibrary.h"

void AGameplayGameMode::BeginPlay()
{
	Super::BeginPlay();

	TArray<AActor*> OutActors;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), APlayerStart::StaticClass(), OutActors);
	for (AActor* CurrentPlayerStart : OutActors)
		AllPlayerStartLocation.Add(CurrentPlayerStart->GetActorLocation());
}

void AGameplayGameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);

	if (AllPlayerStartLocation.Num() < 1)
	{
		FTimerHandle UnusedTimer;
		GetWorldTimerManager().SetTimer(UnusedTimer,
			FTimerDelegate::CreateLambda([this, NewPlayer] {
				SpawnPlayerCharacter(NewPlayer);
				}),
			0.1f,
			false);
	}
	else {
		SpawnPlayerCharacter(NewPlayer);
	}

}

void AGameplayGameMode::SpawnPlayerCharacter(APlayerController* NewPlayer)
{
	if (AllPlayerStartLocation.Num() < 1)
	{
		FTimerHandle UnusedTimer;
		GetWorldTimerManager().SetTimer(UnusedTimer,
			FTimerDelegate::CreateLambda([this, NewPlayer] {
				SpawnPlayerCharacter(NewPlayer);
				}),
			0.1f,
			false);
	}
	else {
		FVector RandomLoc = AllPlayerStartLocation[FMath::RandRange(0, AllPlayerStartLocation.Num() - 1)];
		FActorSpawnParameters SpawnInfo;
		AGameplayCharacter* CharacterRef = GetWorld()->SpawnActor<AGameplayCharacter>(GameplayCharacterClass, RandomLoc, FRotator(0), SpawnInfo);
		int LocOffset = 1;
		while (!CharacterRef)
		{
			CharacterRef = GetWorld()->SpawnActor<AGameplayCharacter>(GameplayCharacterClass, RandomLoc + FVector(0.f, 0.f, 200.f * LocOffset), FRotator(0), SpawnInfo);
			LocOffset++;
		}
		NewPlayer->Possess(CharacterRef);
		CharacterRef->Server_OnSetPlayerController();
	}
}

void AGameplayGameMode::OnBuyWaveExtention()
{
	UDefaultGameInstance* GameInstance = Cast<UDefaultGameInstance>(UGameplayStatics::GetGameInstance(GetWorld()));
	GameInstance->ActivateWaveExtention();
	UGameplayStatics::SetGlobalTimeDilation(GetWorld(), 1.f);
}

void AGameplayGameMode::OnReachLimitWave()
{
	UGameplayStatics::SetGlobalTimeDilation(GetWorld(), 0.f);

	UDefaultGameInstance* GameInstance = Cast<UDefaultGameInstance>(UGameplayStatics::GetGameInstance(GetWorld()));
	if (GameInstance->WaveExtentionActivated)
		return;
	
	AGameplayPlayerController* ControllerRef = Cast<AGameplayPlayerController>(UGameplayStatics::GetPlayerController(GetWorld(), 0));
	ControllerRef->Client_CreateWaveExtentionWidget();
}
