// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ProceduralMeshComponent.h"
#include "GameFramework/Actor.h"
#include "ProceduralTerrainGenerator.generated.h"

class UProceduralMeshComponent;

USTRUCT(BlueprintType)
struct FFoliageToSpawn
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UStaticMesh* Mesh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int Amount;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float ScaleMultiplier;

};

UCLASS()
class CFTWO_API AProceduralTerrainGenerator : public AActor
{
	GENERATED_BODY()

private:
	TArray<FVector> Vertices;
	TArray<int> Triangles;
	TArray<FVector2D> UV0;
	TArray<FVector> Normals;
	TArray<struct FProcMeshTangent> Tangents;

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
	TArray<FFoliageToSpawn> Foliages;

private:
	void GenerateTerrain();

	void CreateVertices();

	void CreateTriangles();

	float Perlin_Noise(float x, float y, float scale = 1.f, float amplitude = 1.f);

	float Interpolate(float a, float b, float x);
	float Fade(float t);
	float dotGradient(int X, float x, int Y = 0, float y = 0, int Z = 0, float z = 0, int W = 0, float w = 0);
	FVector4 Random(int X, int Y, int Z, int W);

	void SpawnFoliage();
	
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	void OnConstruction(const FTransform& Transform);

public:
	// Sets default values for this actor's properties
	AProceduralTerrainGenerator();

};
