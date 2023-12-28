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
	int Amount = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Meta = (ClampMin = 1))
	float ScaleMultiplier = 1;

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
	TArray<FFoliageToSpawn> Foliages;

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

	void OnConstruction(const FTransform& Transform);

public:
	// Sets default values for this actor's properties
	AProceduralTerrainGenerator();

};
