// Fill out your copyright notice in the Description page of Project Settings.


#include "Pickable.h"

// Sets default values
APickable::APickable()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	StaticMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StaticMesh"));
	RootComponent = StaticMesh;
	StaticMesh->SetCollisionObjectType(ECC_WorldStatic);
	StaticMesh->SetSimulatePhysics(true);
}

// Called when the game starts or when spawned
void APickable::BeginPlay()
{
	Super::BeginPlay();
	
	StaticMesh->SetStaticMesh(GetItemInfo(ItemId).Mesh);
}

FInventoryItem APickable::GetItemInfo(const int Index)
{
	return *(ItemsDataTable->FindRow<FInventoryItem>(FName(*(FString::FromInt(Index))), ""));
}

