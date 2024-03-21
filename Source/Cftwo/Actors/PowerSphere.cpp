// Fill out your copyright notice in the Description page of Project Settings.


#include "PowerSphere.h"

#include "BreakableObject.h"
#include "Cftwo/GameplayFramework/GameplayGameMode.h"
#include "Cftwo/GameplayFramework/GameplayGameState.h"
#include "Cftwo/Utils/GeneralFunctionLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"

// Sets default values
APowerSphere::APowerSphere()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void APowerSphere::BeginPlay()
{
	Super::BeginPlay();

	Server_StartSpawnWaves();
}

void APowerSphere::Server_StartSpawnWaves_Implementation()
{
	GameState = Cast<AGameplayGameState>(UGameplayStatics::GetGameState(GetWorld()));
	
	SpawnWave();
}

void APowerSphere::SpawnWave()
{
	FTimerHandle UnusedHandle;
	GetWorldTimerManager().SetTimer(UnusedHandle,
		FTimerDelegate::CreateLambda([this]
		{
			InitialIntervalBetweenWaves = FMath::Clamp(InitialIntervalBetweenWaves + IntervalAdditionBetweenWaves, 0.f, 60.f);
			SpawnWave();
		}),
		InitialIntervalBetweenWaves,
		false
		);
	
	if (!GameState)
	{
		GameState = Cast<AGameplayGameState>(UGameplayStatics::GetGameState(GetWorld()));
		GPrintDebug("found no game state on power sphere. Trying again later.");
		return;
	}

	if (!UGameplayStatics::GetPlayerPawn(GetWorld(), 0))
	{
		GPrintDebug("found no player pawn on power sphere. Trying again later.");
		return;
	}
	
	const int NumberOfMonstersToSpawn = GameState->CurrentWave + 1;
	FVector PawnLocation = UGameplayStatics::GetPlayerPawn(GetWorld(), 0)->GetActorLocation();
	
	for (int i = 0; i < NumberOfMonstersToSpawn; i++)
	{
		const FVector SpawnDirection = GetActorForwardVector();
		const float CurrentSpawnAngle = i*SpawnAngle;
		const float XLocationNotRotated = SpawnDistance * SpawnDirection.X;
		const float YLocationNotRotated = SpawnDistance * SpawnDirection.Y;
		const float SpawnLocationX = GetActorLocation().X + XLocationNotRotated * cos(CurrentSpawnAngle) - YLocationNotRotated * sin(CurrentSpawnAngle);
		const float SpawnLocationY = GetActorLocation().Y + XLocationNotRotated * sin(CurrentSpawnAngle) + YLocationNotRotated * cos(CurrentSpawnAngle);
		const FVector SpawnLocation = FVector(SpawnLocationX, SpawnLocationY, GetActorLocation().Z);
		FRotator SpawnRotation = UKismetMathLibrary::FindLookAtRotation(SpawnLocation, PawnLocation);
		SpawnRotation.Pitch = 0.f;
		SpawnRotation.Roll = 0.f;
		
		FHitResult OutHit;
		GetWorld()->LineTraceSingleByChannel(OutHit,
			SpawnLocation + FVector(0.f, 0.f, 20000),
			SpawnLocation + FVector(0.f, 0.f, -20000),
			ECC_Visibility);
		if (OutHit.bBlockingHit)
		{
			int RandomIndex = FMath::RandRange(0, MonstersToSpawn.Num() - 1);
			GetWorld()->SpawnActor<AActor>(MonstersToSpawn[RandomIndex],
				SpawnLocation + FVector(0.f, 0.f, 200.f),
				SpawnRotation,
				FActorSpawnParameters());
		}
	}
	
	GameState->CurrentWave++;

	if (GameState->CurrentWave == 11)
	{
		AGameplayGameMode* CurrentGameMode = Cast<AGameplayGameMode>(UGameplayStatics::GetGameMode(GetWorld()));
		CurrentGameMode->OnReachLimitWave();
	}
}


