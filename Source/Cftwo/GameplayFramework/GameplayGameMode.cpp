// Fill out your copyright notice in the Description page of Project Settings.

#include "GameplayGameMode.h"
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
		GPrintDebug("failed to spawn");
	}
	else {
		FVector RandomLoc = AllPlayerStartLocation[FMath::RandRange(0, AllPlayerStartLocation.Num() - 1)];
		FActorSpawnParameters SpawnInfo;
		AGameplayCharacter* CharacterRef = GetWorld()->SpawnActor<AGameplayCharacter>(GameplayCharacterClass, RandomLoc, FRotator(0), SpawnInfo);
		NewPlayer->Possess(CharacterRef);
		CharacterRef->Client_InitializeInventory();
		GPrintDebug("spawning");
	}
}
