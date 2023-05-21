// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "BreakableFoliage.generated.h"

UCLASS()
class CFTWO_API ABreakableFoliage : public AActor
{
	GENERATED_BODY()
private:
	// Start with 2 HP and need 3 hits to fall (the first one was made in order to spawn this actor)
	int m_CurrentHealth = 2;

public:
	UPROPERTY(VisibleDefaultsOnly)
		UStaticMeshComponent* m_StaticMesh = nullptr;
	
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Sets default values for this actor's properties
	ABreakableFoliage();

	// Remove a hit point and
	// @returns: Whether or not rock was destroyed.
	bool GetHitted();

};
