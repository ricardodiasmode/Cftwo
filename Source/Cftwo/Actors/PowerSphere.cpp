// Fill out your copyright notice in the Description page of Project Settings.


#include "PowerSphere.h"

// Sets default values
APowerSphere::APowerSphere()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

}

// Called when the game starts or when spawned
void APowerSphere::BeginPlay()
{
	Super::BeginPlay();
	
	SpawnWave();

	FTimerHandle UnusedHandle;
	GetWorldTimerManager().SetTimer(UnusedHandle,
		FTimerDelegate::CreateLambda([this]
		{
			SpawnWave();
		}),
		IntervalBetweenWaves,
		true
		);
	
}

void APowerSphere::SpawnWave()
{
	const int NumberOfMonstersToSpawn = CurrentWave;
	
	for (int i = 0; i < NumberOfMonstersToSpawn; i++)
	{
		const FVector SpawnDirection = GetActorForwardVector();
		const float CurrentSpawnAngle = i*SpawnAngle;
		const float XLocationNotRotated = SpawnDistance * SpawnDirection.X;
		const float YLocationNotRotated = SpawnDistance * SpawnDirection.Y;
		const float SpawnLocationX = GetActorLocation().X + XLocationNotRotated * cos(CurrentSpawnAngle) - YLocationNotRotated * sin(CurrentSpawnAngle);
		const float SpawnLocationY = GetActorLocation().Y + XLocationNotRotated * sin(CurrentSpawnAngle) + YLocationNotRotated * cos(CurrentSpawnAngle);
		const FVector SpawnLocation = FVector(SpawnLocationX, SpawnLocationY, GetActorLocation().Z);
		FHitResult OutHit;
		GetWorld()->LineTraceSingleByChannel(OutHit,
			SpawnLocation + FVector(0.f, 0.f, 5000),
			SpawnLocation + FVector(0.f, 0.f, -5000),
			ECC_Visibility);
		if (OutHit.bBlockingHit)
		{
			int RandomIndex = FMath::RandRange(0, MonstersToSpawn.Num() - 1);
			GetWorld()->SpawnActor<AActor>(MonstersToSpawn[RandomIndex],
				SpawnLocation,
				FRotator(),
				FActorSpawnParameters());
		}
	}
	
	CurrentWave++;
}


