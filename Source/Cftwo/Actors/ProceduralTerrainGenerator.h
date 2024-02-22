// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ProceduralMeshComponent.h"
#include "Engine/DataTable.h"
#include "GameFramework/Actor.h"
#include "ProceduralTerrainGenerator.generated.h"

class UDataTable;
class UProceduralMeshComponent;

USTRUCT(BlueprintType)
struct FFoliageToSpawn
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UStaticMesh* Mesh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int Amount = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Meta = (ClampMin = 1))
	float ScaleMultiplier = 1;

};

USTRUCT(BlueprintType)
struct FProceduralBuilding : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int BuildingMinSizeX = 1;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int BuildingMinSizeY = 1;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TSubclassOf<AActor> BuildingClass;

};

UCLASS()
class CFTWO_API AProceduralTerrainGenerator : public AActor
{
	GENERATED_BODY()

private:
	TArray<FVector> CenterVertices;
	TArray<int> CenterTriangles;
	TArray<FVector2D> CenterUV;
	TArray<FVector> CenterNormals;
	TArray<FProcMeshTangent> CenterTangents;

	float coefs[11];

protected:
	UProceduralMeshComponent* ProceduralMesh = nullptr;

public:
	UPROPERTY(EditAnywhere)
	UMaterialInterface* MaterialRef = nullptr;

	UPROPERTY(EditAnywhere, Meta = (ClampMin = 1))
	int XSize = 1;
	UPROPERTY(EditAnywhere, Meta = (ClampMin = 1))
	int YSize = 1;
	UPROPERTY(EditAnywhere, Meta = (ClampMin = 0.01))
	float Scale = 1;
	UPROPERTY(EditAnywhere, Meta = (ClampMin = 0.1))
	int UVScale = 1;

	UPROPERTY(EditAnywhere)
	float ZMultiplier = 100.f;
	UPROPERTY(EditAnywhere)
	float ZSmoothness = 10.f;
	UPROPERTY(EditAnywhere)
	float EdgeMultiplier = 5.f;
	UPROPERTY(EditAnywhere)
	float EdgeSize = 0.2f;
	UPROPERTY(EditAnywhere)
	float MinimumEdgeMultiplier = 1.5f;
	
	UPROPERTY(EditAnywhere, Meta = (ClampMin = 0))
	int NumberOfBuildings = 1;
	
	UPROPERTY(EditAnywhere, Meta = (ClampMin = 0))
	TArray<int> AllowedBuildings;

	UPROPERTY(EditAnywhere)
	TArray<FFoliageToSpawn> Foliages;

	UPROPERTY(EditDefaultsOnly)
	UDataTable* BuildingsDT;

	UPROPERTY(EditAnywhere)
	bool DebugBuildings = false;
	
	TArray<FProceduralBuilding> Buildings;

	TArray<TPair<FVector, int>> LocationsToSpawnBuildings;

private:
	void GenerateTerrain();

	void CreateVerticesAndTriangles();

	float Perlin_Noise(float x, float y, float scale = 1.f, float amplitude = 1.f);

	float Interpolate(float a, float b, float x);
	float Fade(float t);
	float dotGradient(int X, float x, int Y = 0, float y = 0, int Z = 0, float z = 0, int W = 0, float w = 0);
	FVector4 Random(int X, int Y, int Z, int W);

	void SpawnFoliage();
	
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	FProceduralBuilding GetBuildingInfoByIndex(int Index) const;

	void OnConstruction(const FTransform& Transform);

public:
	// Sets default values for this actor's properties
	AProceduralTerrainGenerator();

};
