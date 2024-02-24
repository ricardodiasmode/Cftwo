// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ProceduralStreet.generated.h"

UCLASS()
class CFTWO_API AProceduralStreet : public AActor
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly)
	UStaticMesh* SplineStaticMesh = nullptr;
	
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly)
	UMaterialInterface* StaticMeshMaterial = nullptr;

	UPROPERTY(BlueprintReadOnly)
	TArray<FVector> SplinePoints;

public:	
	// Sets default values for this actor's properties
	AProceduralStreet();
	
};
