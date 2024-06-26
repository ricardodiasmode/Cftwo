// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "../Utils/GeneralFunctionLibrary.h"
#include "ActorSpawner.generated.h"

class UBoxComponent;

UCLASS()
class CFTWO_API AActorSpawner : public AActor
{
	GENERATED_BODY()
protected:
	UPROPERTY(VisibleAnywhere)
	UBoxComponent* SpawnerBounds = nullptr;

	UPROPERTY(EditAnywhere)
	TArray<struct FActorToSpawn> ActorsToSpawn;

public:
	TArray<FActorMatrix> ActorsSpawned;

private:
	UFUNCTION(Server, Reliable)
	void Server_StartSpawnTimer();

	void CheckShouldSpawn();

	void SpawnAllActors();

	bool SpawnActor(const int Index);
	
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Sets default values for this actor's properties
	AActorSpawner();

	void OnLoseActor(AActor* ActorRef);

};
