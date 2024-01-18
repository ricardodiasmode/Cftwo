// Fill out your copyright notice in the Description page of Project Settings.


#include "Pickable.h"

#include "Cftwo/Utils/GeneralFunctionLibrary.h"
#include "Components/SphereComponent.h"

// Sets default values
APickable::APickable()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	StaticMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StaticMesh"));
	RootComponent = StaticMesh;
	StaticMesh->SetCollisionObjectType(ECC_GameTraceChannel1);
	StaticMesh->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore);
	StaticMesh->SetSimulatePhysics(true);

	SphereCollision = CreateDefaultSubobject<USphereComponent>(TEXT("SphereCollision"));
	SphereCollision->SetupAttachment(RootComponent);
	SphereCollision->SetCollisionObjectType(ECC_GameTraceChannel1);
	SphereCollision->SetSphereRadius(200.f);
	
	for (UActorComponent* CurrentComponent : GetComponents())
	{
		CurrentComponent->SetCanEverAffectNavigation(false);
	}
}

// Called when the game starts or when spawned
void APickable::BeginPlay()
{
	Super::BeginPlay();

	StaticMesh->SetStaticMesh(GetItemInfo(ItemId).Mesh);
}

FInventoryItem APickable::GetItemInfo(const int Index) const
{
	return *(ItemsDataTable->FindRow<FInventoryItem>(FName(*(FString::FromInt(Index))), ""));
}

void APickable::OnPick()
{
	Destroy();
}

