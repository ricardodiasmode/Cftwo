// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PowerSphere.generated.h"

UCLASS()
class CFTWO_API APowerSphere : public AActor
{
	GENERATED_BODY()

private:
	int CurrentWave = 1; 

protected:
	UPROPERTY(EditDefaultsOnly, Meta = (ClampMin = 0.f))
	float IntervalBetweenWaves = 60.f;
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

public:	
	
	void SpawnWave();
};
