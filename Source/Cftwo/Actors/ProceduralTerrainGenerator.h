// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include <vector>

#include "CoreMinimal.h"
#include "ProceduralMeshComponent.h"
#include "Engine/DataTable.h"
#include "GameFramework/Actor.h"
#include "ProceduralTerrainGenerator.generated.h"

class AProceduralStreet;
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

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Meta = (ClampMin = 1))
	float CullDistance = 10000.f;

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
	UPROPERTY(EditAnywhere)
	TSubclassOf<AProceduralStreet> StreetClass;
	
	UPROPERTY(EditAnywhere, Meta = (ClampMin = 0))
	TArray<int> AllowedBuildings;

	UPROPERTY(EditAnywhere)
	TArray<FFoliageToSpawn> Foliages;

	UPROPERTY(EditDefaultsOnly)
	UDataTable* BuildingsDT;

	UPROPERTY(EditAnywhere)
	bool DebugBuildings = false;

	UPROPERTY(EditAnywhere, Meta = (ClampMin = 1.f))
	float BuildingLocationOptimizationJump = 2.f;

	UPROPERTY(EditAnywhere, Meta = (ClampMin = 1.f))
	int EdgeDistanceMultiplier = 4;

	UPROPERTY(EditAnywhere, Meta = (ClampMin = 1.f))
	int MinBuildingVertexDistance = 10;
	
	TArray<FProceduralBuilding> Buildings;

	TArray<TPair<FVector, int>> LocationsToSpawnBuildings;

	//TArray<FVector> StreetLocations;

private:
	void GenerateTerrain();

	void GenerateBuildings(const float LowerXBorder, const float UpperXBorder, const float LowerYBorder, const float UpperYBorder, TArray<TPair<FVector2D, int>>* BuildingLocation);

	//void GenerateStreets(TArray<TPair<FVector2D, int>> BuildingLocation);

	void CreateVerticesAndTriangles();

	float Perlin_Noise(float x, float y, float scale = 1.f, float amplitude = 1.f);

	float Interpolate(float a, float b, float x);
	float Fade(float t);
	float dotGradient(int X, float x, int Y = 0, float y = 0, int Z = 0, float z = 0, int W = 0, float w = 0);
	FVector4 Random(int X, int Y, int Z, int W);

	void SpawnFoliage();

	FVector2D FindRandomLocationFarFromBuildings(const TArray<TPair<FVector2D, int>>& Points, const int BuildingIndex, const float LowerXBorder, const float LowerYBorder, const float UpperXBorder, const float UpperYBorder);
	
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	FProceduralBuilding GetBuildingInfoByIndex(int Index) const;

	void OnConstruction(const FTransform& Transform);

public:
	// Sets default values for this actor's properties
	AProceduralTerrainGenerator();

	static float GenerateRandomFloat(float min, float max);
};
