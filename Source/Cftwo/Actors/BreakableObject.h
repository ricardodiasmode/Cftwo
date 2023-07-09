// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "BreakableObject.generated.h"

UCLASS()
class CFTWO_API ABreakableObject : public AActor
{
	GENERATED_BODY()

private:
	// Means that needs {CurrentHP} value to break. Must be decreased by RemoveHP() function.
	int CurrentHP = 3;

public:
	class UStaticMeshComponent* StaticMeshComponent = nullptr;
	
private:
	void Break();	

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Sets default values for this actor's properties
	ABreakableObject();

	void RemoveHP();
};
