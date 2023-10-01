// Fill out your copyright notice in the Description page of Project Settings.


#include "ProceduralTerrainGenerator.h"
#include "KismetProceduralMeshLibrary.h"
#include "stdlib.h"
#include "time.h"
#include "math.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "../Utils/GeneralFunctionLibrary.h"

// Sets default values
AProceduralTerrainGenerator::AProceduralTerrainGenerator()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	ProceduralMesh = CreateDefaultSubobject<UProceduralMeshComponent>(TEXT("ProceduralMesh"));

}

void AProceduralTerrainGenerator::GenerateTerrain()
{
	CreateVertices();
	CreateTriangles();

	UKismetProceduralMeshLibrary::CalculateTangentsForMesh(Vertices, Triangles, UV0, Normals, Tangents);

	ProceduralMesh->ClearMeshSection(0);
	ProceduralMesh->CreateMeshSection(0, Vertices, Triangles, Normals, UV0, TArray<FColor>(), Tangents, true);
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

	SpawnFoliage();
}

void AProceduralTerrainGenerator::CreateVertices()
{
	Vertices.Reset();
	UV0.Reset();

	for (int i = 0; i <= XSize; i++)
	{
		for (int j = 0; j <= YSize; j++)
		{
			float XAsFloat = static_cast<float>(i);
			float YAsFloat = static_cast<float>(j);
			Vertices.Add(FVector(i * Scale, j * Scale, Perlin_Noise(XAsFloat, YAsFloat, ZSmoothness)*ZMultiplier));
			UV0.Add(FVector2D(i * UVScale, j * UVScale));
		}
	}
}

void AProceduralTerrainGenerator::CreateTriangles()
{
	Triangles.Reset();

	int CurrentVertex = 0;

	for (int i = 0; i < XSize; i++)
	{
		for (int j = 0; j < YSize; j++)
		{
			Triangles.Add(CurrentVertex);
			Triangles.Add(CurrentVertex + 1);
			Triangles.Add(CurrentVertex + YSize + 1);
			Triangles.Add(CurrentVertex + 1);
			Triangles.Add(CurrentVertex + YSize + 2);
			Triangles.Add(CurrentVertex + YSize + 1);

			CurrentVertex++;
		}
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
			DrawDebugLine(GetWorld(), StartWorldSpawnPoint, EndWorldSpawnPoint, FColor::Red, true);
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
				DrawDebugLine(GetWorld(), LocationToSpawn, LocationToSpawn + FVector(0.f, 0.f, 300.f), FColor::Green, true);

				FRandomStream RandStream;
				FRotator RotatorToSpawn = FRotationMatrix::MakeFromZX(OutHit.Normal,
					RandStream.GetUnitVector()).Rotator();

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

