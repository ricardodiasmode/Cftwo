// Fill out your copyright notice in the Description page of Project Settings.


#include "Pickable.h"

#include "ActorSpawner.h"
#include "Cftwo/GameplayFramework/Characters/GameplayCharacter.h"
#include "Cftwo/Utils/GeneralFunctionLibrary.h"
#include "Components/SphereComponent.h"

// Sets default values
APickable::APickable()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;
	PrimaryActorTick.bStartWithTickEnabled = false;

	StaticMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StaticMesh"));
	RootComponent = StaticMesh;
	StaticMesh->SetCollisionObjectType(ECC_GameTraceChannel1);
	StaticMesh->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore);
	StaticMesh->SetSimulatePhysics(true);
	StaticMesh->PrimaryComponentTick.bCanEverTick = false;
	
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
	StaticMesh->SetCullDistance(GetItemInfo(ItemId).CullDistance);

	if (SpawnerRef)
	{
		for (auto [ActorArray] : SpawnerRef->ActorsSpawned)
		{
			if (ActorArray.Num() == 0)
				return;
		
			if (Cast<AGameplayCharacter>(ActorArray[0]))
			{
				for (AActor* CurrentNPC : ActorArray)
					Cast<AGameplayCharacter>(CurrentNPC)->Pickables.Add(this);
			}
		}
	}
}

FInventoryItem APickable::GetItemInfo(const int Index) const
{
	return *(ItemsDataTable->FindRow<FInventoryItem>(FName(*(FString::FromInt(Index))), ""));
}

void APickable::OnPick()
{
	if (SpawnerRef)
	{
		for (auto [ActorArray] : SpawnerRef->ActorsSpawned)
		{
			if (ActorArray.Num() == 0)
				return;

			if (!IsValid(ActorArray[0]))
				return;

			if (ActorArray[0]->StaticClass() == AGameplayCharacter::StaticClass())
			{
				for (AActor* CurrentNPC : ActorArray)
					Cast<AGameplayCharacter>(CurrentNPC)->Pickables.Remove(this);
			}
		}
	}
	
	Destroy();
}

