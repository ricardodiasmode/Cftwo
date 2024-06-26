// Fill out your copyright notice in the Description page of Project Settings.


#include "ActorSpawner.h"

#include "BreakableObject.h"
#include "Pickable.h"
#include "Components/BoxComponent.h"
#include "../Utils/ActorToSpawn.h"
#include "Kismet/KismetMathLibrary.h"
#include "Cftwo/GameplayFramework/AI/BaseNeutralCharacter.h"
#include "Cftwo/GameplayFramework/Characters/GameplayCharacter.h"
#include "EntitySystem/MovieSceneEntitySystemRunner.h"
#include "Kismet/GameplayStatics.h"

// Sets default values
AActorSpawner::AActorSpawner()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	SpawnerBounds = CreateDefaultSubobject<UBoxComponent>(TEXT("SpawnerBounds"));
	RootComponent = SpawnerBounds;
	
	for (UActorComponent* CurrentComponent : GetComponents())
	{
		CurrentComponent->SetCanEverAffectNavigation(false);
	}
}

// Called when the game starts or when spawned
void AActorSpawner::BeginPlay()
{
	Super::BeginPlay();

	// Calling spawn with a delay to make sure terrain is generated
	FTimerHandle UnusedHandle;
	GetWorldTimerManager().SetTimer(UnusedHandle, FTimerDelegate::CreateLambda( [this]
	{
		Server_StartSpawnTimer();

		SpawnAllActors();
	}),
	1.f,
	false);
}

void AActorSpawner::Server_StartSpawnTimer_Implementation()
{
	FTimerHandle UnusedHandle;
	GetWorldTimerManager().SetTimer(UnusedHandle, this, &AActorSpawner::CheckShouldSpawn, 1.f, true, 0.1f);
}

void AActorSpawner::SpawnAllActors()
{
	for (int i = 0; i < ActorsToSpawn.Num(); i++)
	{
		ActorsSpawned.Add(FActorMatrix());

		for (int j=0; j < ActorsToSpawn[i].MinimumSpawned; j++)
		{
			SpawnActor(i);
		}
	}
}

void AActorSpawner::CheckShouldSpawn()
{
	for (int i=0;i < ActorsToSpawn.Num();i++)
	{
		// Check if we have actors enough
		if (ActorsSpawned.Num() > i)
		{
			if (ActorsSpawned[i].ActorArray.Num() >= ActorsToSpawn[i].MinimumSpawned ||
				!ActorsToSpawn[i].CanRespawn)
				continue;
		}

		while(!SpawnActor(i));
	}
}

bool AActorSpawner::SpawnActor(const int Index)
{
	FVector StartLocation = UKismetMathLibrary::RandomPointInBoundingBox(SpawnerBounds->GetComponentLocation(),
		SpawnerBounds->GetLocalBounds().BoxExtent);
	StartLocation.Z = GetActorLocation().Z + 10000.f;
	FVector EndLocation = StartLocation + FVector(0.f, 0.f, -20000.f);

	FHitResult OutHit;
	if (GetWorld()->LineTraceSingleByChannel(OutHit,
		StartLocation, EndLocation,
		ECollisionChannel::ECC_Visibility))
	{
		if (Cast<ABreakableObject>(OutHit.GetActor()))
			return false;
		
		TSubclassOf<AActor> ClassToSpawn = ActorsToSpawn[Index].ActorClass;
		FVector SpawnLoc = OutHit.ImpactPoint;
		if (ActorsToSpawn[Index].Foliage)
			SpawnLoc -= FVector(0.f, 0.f, 30.f);
		else
			SpawnLoc += FVector(0.f, 0.f, 100.f);
		FTransform SpawnTransform(FRotator(), SpawnLoc, FVector(1.f, 1.f, 1.f));

		FActorSpawnParameters SpawnParams;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
		AActor* ActorRef = GetWorld()->SpawnActorDeferred<AActor>(ClassToSpawn, SpawnTransform);
		if (Cast<ABreakableObject>(ActorRef))
			Cast<ABreakableObject>(ActorRef)->SpawnerRef = this;
		else if (Cast<AGameplayCharacter>(ActorRef))
			Cast<AGameplayCharacter>(ActorRef)->SpawnerRef = this;
		else if (Cast<ABaseNeutralCharacter>(ActorRef))
			Cast<ABaseNeutralCharacter>(ActorRef)->SpawnerRef = this;
		else if (Cast<APickable>(ActorRef))
			Cast<APickable>(ActorRef)->SpawnerRef = this;
		ActorsSpawned[Index].ActorArray.Add(ActorRef);
		UGameplayStatics::FinishSpawningActor(ActorRef, SpawnTransform);
	}
	return true;
}

void AActorSpawner::OnLoseActor(AActor* ActorRef)
{
	for (int i = 0; i < ActorsSpawned.Num(); i++)
	{
		if (ActorsSpawned[i].ActorArray.Contains(ActorRef))
		{
			ActorsSpawned[i].ActorArray.Remove(ActorRef);
			return;
		}
	}
}
