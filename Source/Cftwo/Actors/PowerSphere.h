// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PowerSphere.generated.h"

class AGameplayGameState;

UCLASS()
class CFTWO_API APowerSphere : public AActor
{
	GENERATED_BODY()

private:
	TObjectPtr<AGameplayGameState> GameState = nullptr;

protected:
	UPROPERTY(EditDefaultsOnly, Meta = (ClampMin = 0.f))
	float InitialIntervalBetweenWaves = 5.f;
	UPROPERTY(EditDefaultsOnly, Meta = (ClampMin = 0.f))
	float IntervalAdditionBetweenWaves = 5.f;
	UPROPERTY(EditDefaultsOnly, Meta = (ClampMin = 0.f))
	float SpawnAngle = 5.f;
	UPROPERTY(EditDefaultsOnly, Meta = (ClampMin = 0.f))
	float SpawnDistance = 1500.f;

	UPROPERTY(EditDefaultsOnly)
	TArray<TSubclassOf<AActor>> MonstersToSpawn;
	
public:	
	// Sets default values for this actor's properties
	APowerSphere();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UFUNCTION(Server, reliable)
	void Server_StartSpawnWaves();

public:	
	
	void SpawnWave();
};
