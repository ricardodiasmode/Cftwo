// Fill out your copyright notice in the Description page of Project Settings.


#include "ProceduralTerrainGenerator.h"
#include "KismetProceduralMeshLibrary.h"
#include "stdlib.h"
#include "time.h"
#include "math.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "../Utils/GeneralFunctionLibrary.h"
#include "Engine/DataTable.h"

// Sets default values
AProceduralTerrainGenerator::AProceduralTerrainGenerator()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	ProceduralMesh = CreateDefaultSubobject<UProceduralMeshComponent>(TEXT("ProceduralMesh"));

}

void AProceduralTerrainGenerator::GenerateTerrain()
{
	CreateVerticesAndTriangles();

	UKismetProceduralMeshLibrary::CalculateTangentsForMesh(CenterVertices, CenterTriangles, CenterUV, CenterNormals, CenterTangents);

	ProceduralMesh->ClearMeshSection(0);
	ProceduralMesh->CreateMeshSection(0, CenterVertices, CenterTriangles, CenterNormals, CenterUV, TArray<FColor>(), CenterTangents, true);
	ProceduralMesh->SetMaterial(0, MaterialRef);
}

void AProceduralTerrainGenerator::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

	GenerateTerrain();
}

// Called when the game starts or when spawned
void AProceduralTerrainGenerator::BeginPlay()
{
	Super::BeginPlay();

	GenerateTerrain();

	SpawnFoliage();

	for (auto [Location, Index] : LocationsToSpawnBuildings)
	{
		GetWorld()->SpawnActor<AActor>(GetBuildingInfoByIndex(Index).BuildingClass, Location, FRotator(0.f));
	}
}

FProceduralBuilding AProceduralTerrainGenerator::GetBuildingInfoByIndex(const int Index) const
{
	return *(BuildingsDT->FindRow<FProceduralBuilding>(FName(*(FString::FromInt(Index))), ""));
}

void AProceduralTerrainGenerator::CreateVerticesAndTriangles()
{
	if (!BuildingsDT)
		return;
	
	CenterVertices.Reset();
	CenterUV.Reset();
	CenterTriangles.Reset();

	int CurrentVertex = 0;

	const float LowerXBorder = static_cast<float>(XSize) * EdgeSize;
	const float UpperXBorder = static_cast<float>(XSize) - LowerXBorder;
	const float LowerYBorder = static_cast<float>(YSize) * EdgeSize;
	const float UpperYBorder = static_cast<float>(YSize) - LowerYBorder;

	TArray<TPair<FVector2D, int>> BuildingLocation;
	if (AllowedBuildings.Num() > 0)
	{
		for (int CurrentBuildingIndex = 0; CurrentBuildingIndex < AllowedBuildings.Num(); CurrentBuildingIndex++)
		{
			int BuildingIndex = AllowedBuildings[CurrentBuildingIndex];
			for (int i = 0; i < NumberOfBuildings; i++)
			{
				const int DesiredX = FMath::RandRange(FMath::Floor(LowerXBorder), FMath::Floor(UpperXBorder - GetBuildingInfoByIndex(BuildingIndex).BuildingMinSizeX));
				const int DesiredY = FMath::RandRange(FMath::Floor(LowerYBorder), FMath::Floor(UpperYBorder - GetBuildingInfoByIndex(BuildingIndex).BuildingMinSizeY));

				// todo: verificar se na location randomizada já tem uma building
				
				FVector2D Location = FVector2D(static_cast<float>(DesiredX), static_cast<float>(DesiredY));
				BuildingLocation.Add({Location, BuildingIndex});
				LocationsToSpawnBuildings.Add({FVector(DesiredX * Scale, DesiredY * Scale, 0), BuildingLocation[BuildingIndex].Value});
			}
		}
	}

	for (int i = 0; i <= XSize; i++)
	{
		for (int j = 0; j <= YSize; j++)
		{
			// Creating vertices
			const float XAsFloat = static_cast<float>(i);
			const float YAsFloat = static_cast<float>(j);
			float ZVertex = Perlin_Noise(XAsFloat, YAsFloat, ZSmoothness) * ZMultiplier;

			// Make sure the border will be higher than the center
			if (i < LowerXBorder || i > UpperXBorder ||
				j < LowerYBorder || j > UpperYBorder)
			{
				const float MaxCenterHeight = ZMultiplier / ZSmoothness;
				const float DesiredValue = abs(ZVertex * EdgeMultiplier);
				const float EdgeHeightMultiplier = FMath::RandRange(MinimumEdgeMultiplier, MinimumEdgeMultiplier*2.f);
				ZVertex = FMath::Max(DesiredValue, MaxCenterHeight * EdgeHeightMultiplier); // Must greater than max center height
			}

			// Setting building height
			for (int CurrentBuilding = 0; CurrentBuilding < BuildingLocation.Num(); CurrentBuilding++)
			{
				const FProceduralBuilding SpawningBuildingInfo = GetBuildingInfoByIndex(BuildingLocation[CurrentBuilding].Value);
				const FVector2D CurrentBuildingLoc = BuildingLocation[CurrentBuilding].Key;
				if (FMath::IsWithin(i, CurrentBuildingLoc.X, CurrentBuildingLoc.X + SpawningBuildingInfo.BuildingMinSizeX) &&
					FMath::IsWithin(j, CurrentBuildingLoc.Y, CurrentBuildingLoc.Y + SpawningBuildingInfo.BuildingMinSizeY))
				{
					ZVertex = 0;
				}
			}

			CenterVertices.Add(FVector(i * Scale, j * Scale, ZVertex));
			CenterUV.Add(FVector2D(i * UVScale, j * UVScale));

			// Creating triangles
			if (i < XSize && j < YSize)
			{
				CenterTriangles.Add(CurrentVertex);
				CenterTriangles.Add(CurrentVertex + 1);
				CenterTriangles.Add(CurrentVertex + YSize + 1);
				CenterTriangles.Add(CurrentVertex + 1);
				CenterTriangles.Add(CurrentVertex + YSize + 2);
				CenterTriangles.Add(CurrentVertex + YSize + 1);
				CurrentVertex++;
			}
		}
		if (i < XSize)
			CurrentVertex++;
	}
}

float AProceduralTerrainGenerator::Perlin_Noise(float x, float y, float scale, float amplitude) 
{
	srand(time(NULL));
	for (int i = 0; i < 11; i++) {
		coefs[i] = rand() * 1000;
	}

	scale = scale <= 0 ? 1 : scale;
	x /= scale;
	y /= scale;
	int xL = floor(x);
	int xU = xL + 1;
	int yL = floor(y);
	int yU = yL + 1;

	float dx = Fade(x - xL);
	float dy = Fade(y - yL);

	return Interpolate(Interpolate(dotGradient(xL, x, yL, y), dotGradient(xU, x, yL, y), dx),
		Interpolate(dotGradient(xL, x, yU, y), dotGradient(xU, x, yU, y), dx), dy) * amplitude;
}

FVector4 AProceduralTerrainGenerator::Random(int X, int Y, int Z, int W) {
	float seed = coefs[0] * sin(coefs[1] * X + coefs[2] * Y + coefs[3] * Z + coefs[4] * W + coefs[5]) * cos(coefs[6] * X + coefs[7] * Y + coefs[8] * Z + coefs[9] * W + coefs[10]);
	srand(seed);
	float x = static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / 2.0)) - 1.0;
	float y = static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / 2.0)) - 1.0;
	float z = static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / 2.0)) - 1.0;
	float w = static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / 2.0)) - 1.0;
	FVector4 random_vector = FVector4(x, y, z, w);
	return random_vector / random_vector.Size();
}

float AProceduralTerrainGenerator::Interpolate(float a, float b, float x) {
	float t = x * PI;
	float f = (1 - cos(t)) * 0.5;
	return a * (1 - f) + b * f;
}

float AProceduralTerrainGenerator::Fade(float t) {
	return t * t * t * (t * (t * 6 - 15) + 10);
}

float AProceduralTerrainGenerator::dotGradient(int X, float x, int Y, float y, int Z, float z, int W, float w) {
	FVector4 random_vector = Random(X, Y, Z, W);
	float dx = x - X;
	float dy = y - Y;
	float dz = z - Z;
	float dw = w - W;
	return dx * random_vector.X + dy * random_vector.Y + dz * random_vector.Z + dw * random_vector.W;
}

void AProceduralTerrainGenerator::SpawnFoliage()
{
	const FVector CenterLocation = GetActorLocation() + FVector(XSize * Scale/2, YSize * Scale/2, 0.f);
	for (FFoliageToSpawn CurrentFoliage : Foliages)
	{
		UInstancedStaticMeshComponent* InstancedComponentRef = NewObject<UInstancedStaticMeshComponent>(this);
		InstancedComponentRef->RegisterComponent();
		InstancedComponentRef->SetStaticMesh(CurrentFoliage.Mesh);
		InstancedComponentRef->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		InstancedComponentRef->SetGenerateOverlapEvents(true);
		InstancedComponentRef->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Block);
		InstancedComponentRef->SetCollisionObjectType(ECollisionChannel::ECC_WorldStatic);
		InstancedComponentRef->SetCullDistances(0, 10000);
		AddInstanceComponent(InstancedComponentRef);

		for (int i = 0; i < CurrentFoliage.Amount; i++)
		{
			const auto Radius = FMath::Min(XSize/2 * Scale, YSize / 2 * Scale);
			const float CollisionTraceRange = ZMultiplier * 4;

			float X = FMath::FRandRange(-Radius, Radius);
			float Y = FMath::FRandRange(-Radius, Radius);
			float Z = FMath::FRandRange(-5.f, 0.f);

			FVector LocalSpawnPoint = FVector(X, Y, Z);

			FVector StartWorldSpawnPoint = CenterLocation + LocalSpawnPoint + FVector(0, 0, CollisionTraceRange);
			FVector EndWorldSpawnPoint = StartWorldSpawnPoint - FVector(0, 0, CollisionTraceRange * 2);

			FHitResult OutHit;
			if (GetWorld()->LineTraceSingleByChannel(OutHit,
				StartWorldSpawnPoint, EndWorldSpawnPoint,
				ECollisionChannel::ECC_Visibility) &&
				OutHit.GetComponent() == ProceduralMesh)
			{
				static constexpr auto VerticalOffset = 1.f;
				static constexpr auto ScaleMin = 0.8f;
				static constexpr auto ScaleMax = 1.25f;

				float RandFloat = FMath::FRandRange(VerticalOffset * -2, VerticalOffset);

				FVector LocationToSpawn = OutHit.Location + OutHit.Normal * RandFloat;

				const float RandPitch = FMath::FRandRange(0.f, 2.5f);
				const float RandYaw = FMath::FRandRange(0.f, 359.f);
				const float RandRoll = FMath::FRandRange(0.f, 2.5f);
				FRotator RotatorToSpawn = FRotator(RandPitch, RandYaw, RandRoll);

				float XScale = FMath::FRandRange(ScaleMin, ScaleMax);
				float YScale = FMath::FRandRange(ScaleMin, ScaleMax);
				float ZScale = FMath::FRandRange(ScaleMin, ScaleMax);
				FVector ScaleToSpawn = FVector(XScale, YScale, ZScale);
				ScaleToSpawn *= CurrentFoliage.ScaleMultiplier;

				FTransform TransformToSpawn(RotatorToSpawn, LocationToSpawn, ScaleToSpawn);

				InstancedComponentRef->AddInstance(TransformToSpawn);
			}
		}
	}
}

