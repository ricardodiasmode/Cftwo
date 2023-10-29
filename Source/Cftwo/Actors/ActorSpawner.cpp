// Fill out your copyright notice in the Description page of Project Settings.


#include "ActorSpawner.h"
#include "Components/BoxComponent.h"
#include "../Utils/ActorToSpawn.h"
#include "Kismet/KismetMathLibrary.h"

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
	
	Server_StartSpawnTimer();
}

// Called every frame
void AActorSpawner::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AActorSpawner::Server_StartSpawnTimer_Implementation()
{
	FTimerHandle UnusedHandle;
	GetWorldTimerManager().SetTimer(UnusedHandle, this, &AActorSpawner::CheckShouldSpawn, 1.f, true, 0.1f);
}

void AActorSpawner::CheckShouldSpawn()
{
	for (int i=0;i < ActorsToSpawn.Num();i++)
	{
		// Check if we have actors enough
		if (ActorsSpawned.Num() - 1 < i)
		{
			ActorsSpawned.Add(FActorMatrix());
		}
		else {
			if (ActorsSpawned[i].ActorArray.Num() - 1 > ActorsToSpawn[i].MinimumSpawned)
				continue;
		}

		FVector StartLocation = UKismetMathLibrary::RandomPointInBoundingBox(SpawnerBounds->GetComponentLocation(),
			SpawnerBounds->GetLocalBounds().BoxExtent);
		StartLocation.Z = GetActorLocation().Z + 10000.f;
		FVector EndLocation = StartLocation + FVector(0.f, 0.f, -20000.f);

		FHitResult OutHit;
		if (GetWorld()->LineTraceSingleByChannel(OutHit,
			StartLocation, EndLocation,
			ECollisionChannel::ECC_Visibility))
		{
			TSubclassOf<AActor> ClassToSpawn = ActorsToSpawn[i].ActorClass;
			FTransform SpawnTransform(FRotator(), OutHit.ImpactPoint + FVector(0.f, 0.f, 100.f), FVector(1.f, 1.f, 1.f));
			ActorsSpawned[i].ActorArray.Add(
				GetWorld()->SpawnActor<AActor>(ClassToSpawn, SpawnTransform)
			);
		}
	}
}
