// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "SpawnableActor.h"
#include "GameFramework/Actor.h"
#include "BreakableObject.generated.h"

UCLASS()
class CFTWO_API ABreakableObject : public AActor
{
	GENERATED_BODY()

private:
	// Means that needs {CurrentHP} value to break. Must be decreased by RemoveHP() function.
	int CurrentHP = 8;

	FTimerHandle ShakeStartTimerHandle;
	FTimerHandle ShakeFinishTimerHandle;

	FRotator InitialMeshRotation;
	
public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
		class UStaticMeshComponent* StaticMeshComponent = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	class UStaticMesh* StaticMeshRef = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	int ItemToGive = -1;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	int NecessaryTool = -1;
	
	class AActorSpawner* SpawnerRef = nullptr;

	UPROPERTY(EditDefaultsOnly)
	float ShakeDuration = 0.3f;

	UPROPERTY(EditDefaultsOnly)
	float ShakeInterval = 0.05f;

	UPROPERTY(EditDefaultsOnly)
	float ShakeIntensity = 1.5f;
	
private:
	void Break();	

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Sets default values for this actor's properties
	ABreakableObject();

	void ShakeOnGetHitted();
	
	void RemoveHP();

	void OnDie();
};
