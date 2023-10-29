// Fill out your copyright notice in the Description page of Project Settings.


#include "ActorSpawner.h"
#include "Components/BoxComponent.h"
#include "../Utils/ActorToSpawn.h"

// Sets default values
AActorSpawner::AActorSpawner()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	SpawnerBounds = CreateDefaultSubobject<UBoxComponent>(TEXT("SpawnerBounds"));
	RootComponent = SpawnerBounds;
}

// Called when the game starts or when spawned
void AActorSpawner::BeginPlay()
{
	Super::BeginPlay();
	
	StartSpawnTimer();
}

// Called every frame
void AActorSpawner::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AActorSpawner::StartSpawnTimer()
{
	FTimerHandle UnusedHandle;
	GetWorldTimerManager().SetTimer(UnusedHandle, this, &AActorSpawner::CheckShouldSpawn, 1.f, true, 0.1f);
}

void AActorSpawner::CheckShouldSpawn()
{

	// Check if thee minimum of spawned actors are there
	for (ActorsToSpawn)
	{
		// if not, spawn it
		FVector StartLocation = UKismetMathLibrary::RandomPointInBoundingBox(SpawnerBounds->GetComponentLocation(),
			SpawnerBounds->GetLocalBounds());
		StartLocation.Z = GetActorLocation.Z + 10000.f;
		FVector EndLocation = StartLocation - FVector(0.f, 0.f, -20000.f);

		// do trace to hit ground
	}
}
