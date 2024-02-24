// Fill out your copyright notice in the Description page of Project Settings.


#include "ProceduralTerrainGenerator.h"
#include "KismetProceduralMeshLibrary.h"
#include "stdlib.h"
#include "time.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "Engine/DataTable.h"
#include <chrono>
#include <random>
#include <cmath>
#include "ProceduralStreet.h"
#include "Kismet/GameplayStatics.h"

// Sets default values
AProceduralTerrainGenerator::AProceduralTerrainGenerator()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	ProceduralMesh = CreateDefaultSubobject<UProceduralMeshComponent>(TEXT("ProceduralMesh"));

}

float AProceduralTerrainGenerator::GenerateRandomFloat(float min, float max) {
	const unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();

	std::mt19937 generator(seed);

	const std::uniform_real_distribution<float> distribution(min, max);

	return distribution(generator);
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

	if (StreetLocations.Num() > 1)
	{
		const FTransform SpawnTransform = FTransform(FRotator(), FVector(StreetLocations[0].X * Scale, StreetLocations[0].Y * Scale, 0.f), FVector(1.f));
		AProceduralStreet* StreetRef = GetWorld()->SpawnActorDeferred<AProceduralStreet>(StreetClass, SpawnTransform);
		if (!StreetRef)
			return;
		StreetRef->SplinePoints.Add(SpawnTransform.GetLocation());
		
		for (int i=1;i< StreetLocations.Num();i += 2)
		{
			FVector2D CurrentStreetLocation = StreetLocations[i] * Scale;
			CurrentStreetLocation -= (StreetLocations[i] * Scale - StreetLocations[i-1] * Scale)/2;
			StreetRef->SplinePoints.Add(FVector(CurrentStreetLocation.X, CurrentStreetLocation.Y, 2.f));
		}
		UGameplayStatics::FinishSpawningActor(StreetRef, SpawnTransform);
	}
}

FProceduralBuilding AProceduralTerrainGenerator::GetBuildingInfoByIndex(const int Index) const
{
	return *(BuildingsDT->FindRow<FProceduralBuilding>(FName(*(FString::FromInt(Index))), ""));
}

FVector2D AProceduralTerrainGenerator::FindRandomLocationFarFromBuildings(const TArray<TPair<FVector2D, int>>& Points, const int BuildingIndex, const float LowerXBorder, const float LowerYBorder, const float UpperXBorder, const float UpperYBorder)
{
	int NumberOfTries = 0;
	FVector2D FoundLocation(0.f);
	bool LeaveLoop = false;
	while (!LeaveLoop)
	{
		if (NumberOfTries > 100)
		{
			DrawDebugSphere(GetWorld(),
				FVector(FoundLocation.X * Scale, FoundLocation.Y * Scale, 0.f),
				3000.f,
				16,
				FColor::Black,
				false,
				5.f,
				0,
				50.f);
			return FoundLocation;
		}
		
		NumberOfTries++;
		const float FirstDesiredX = FMath::Floor(GenerateRandomFloat(LowerXBorder, UpperXBorder - GetBuildingInfoByIndex(AllowedBuildings[BuildingIndex]).BuildingMinSizeX));
		const float FirstDesiredY = FMath::Floor(GenerateRandomFloat(LowerYBorder, UpperYBorder - GetBuildingInfoByIndex(AllowedBuildings[BuildingIndex]).BuildingMinSizeY));
		FVector2D Location = FVector2D(FirstDesiredX, FirstDesiredY);
		
		bool Found = true;
		for (auto [Point, unused] : Points)
		{
			if (FVector2D::Distance(Location, Point) < static_cast<float>(MinBuildingVertexDistance))
			{
				Found = false;
				break;
			}
		}
		if (Found)
		{
			LeaveLoop = true;
			FoundLocation = Location;
		}
	}
	return FoundLocation;
}

void AProceduralTerrainGenerator::GenerateBuildings(const float LowerXBorder, const float UpperXBorder, const float LowerYBorder, const float UpperYBorder, TArray<TPair<FVector2D, int>>* BuildingLocation)
{
	if (AllowedBuildings.Num() > 0)
	{
		// Adding first
		const float FirstDesiredX = FMath::Floor(GenerateRandomFloat(LowerXBorder, UpperXBorder - GetBuildingInfoByIndex(AllowedBuildings[0]).BuildingMinSizeX));
		const float FirstDesiredY = FMath::Floor(GenerateRandomFloat(LowerYBorder, UpperYBorder - GetBuildingInfoByIndex(AllowedBuildings[0]).BuildingMinSizeY));
		FVector2D Location = FVector2D(FirstDesiredX, FirstDesiredY);
		BuildingLocation->Add({Location, AllowedBuildings[0]});
		LocationsToSpawnBuildings.Add({FVector(FirstDesiredX * Scale, FirstDesiredY * Scale, 0), (*BuildingLocation)[AllowedBuildings[0]].Value});
		
		// Adding others
		for (int CurrentBuildingIndex = 1; CurrentBuildingIndex < AllowedBuildings.Num(); CurrentBuildingIndex++)
		{
			int BuildingIndex = AllowedBuildings[CurrentBuildingIndex];
			Location = FindRandomLocationFarFromBuildings(*BuildingLocation, BuildingIndex, LowerXBorder, LowerYBorder, UpperXBorder, UpperYBorder);
			BuildingLocation->Add({Location, BuildingIndex});
			LocationsToSpawnBuildings.Add({FVector(Location.X * Scale, Location.Y * Scale, 0), (*BuildingLocation)[BuildingIndex].Value});
		}
	}
}

void AProceduralTerrainGenerator::GenerateStreets(TArray<TPair<FVector2D, int>> BuildingLocation)
{
	StreetLocations.Empty();
	for (int i=1;i<BuildingLocation.Num();i++)
	{
		auto [FirstLocation, FirstUnused] = BuildingLocation[i-1];
		auto [SecondLocation, SecondUnused] = BuildingLocation[i];
		
		FVector2D Path = FirstLocation;
		while(Path != SecondLocation)
		{
			FVector2D Direction(0.f);
			if (SecondLocation.X > Path.X)
				Direction.X = 1;
			else if (SecondLocation.X == Path.X)
			{
				if (SecondLocation.Y > Path.Y)
					Direction.Y = 1;
				else if (SecondLocation.Y == Path.Y)
					Direction.Y = 0;
				else
					Direction.Y = -1;
			}
			else
				Direction.X = -1;
			
			Path += Direction;
			StreetLocations.Add(Path);
			StreetLocations.Add(Path + Direction.GetRotated(90));
		}
	}
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
	GenerateBuildings(LowerXBorder, UpperXBorder, LowerYBorder, UpperYBorder, &BuildingLocation);

	GenerateStreets(BuildingLocation);
	
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
				const float EdgeHeightMultiplier = GenerateRandomFloat(MinimumEdgeMultiplier, MinimumEdgeMultiplier*2.f);
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

			if (StreetLocations.Contains(FVector2D(i, j)))
				ZVertex = 0;

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

			float X = GenerateRandomFloat(-Radius, Radius);
			float Y = GenerateRandomFloat(-Radius, Radius);
			float Z = GenerateRandomFloat(-5.f, 0.f);

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

				float RandFloat = GenerateRandomFloat(VerticalOffset * -2, VerticalOffset);

				FVector LocationToSpawn = OutHit.Location + OutHit.Normal * RandFloat;

				const float RandPitch = GenerateRandomFloat(0.f, 2.5f);
				const float RandYaw = GenerateRandomFloat(0.f, 359.f);
				const float RandRoll = GenerateRandomFloat(0.f, 2.5f);
				FRotator RotatorToSpawn = FRotator(RandPitch, RandYaw, RandRoll);

				float XScale = GenerateRandomFloat(ScaleMin, ScaleMax);
				float YScale = GenerateRandomFloat(ScaleMin, ScaleMax);
				float ZScale = GenerateRandomFloat(ScaleMin, ScaleMax);
				FVector ScaleToSpawn = FVector(XScale, YScale, ZScale);
				ScaleToSpawn *= CurrentFoliage.ScaleMultiplier;

				FTransform TransformToSpawn(RotatorToSpawn, LocationToSpawn, ScaleToSpawn);

				InstancedComponentRef->AddInstance(TransformToSpawn);
			}
		}
	}
}

