// Fill out your copyright notice in the Description page of Project Settings.


#include "Chest.h"

#include "Cftwo/GameplayFramework/Components/Inventory/InventorySlot.h"
#include "Net/UnrealNetwork.h"

// Sets default values
AChest::AChest()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

}

void AChest::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const 
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AChest, Slots);
}

// Called when the game starts or when spawned
void AChest::BeginPlay()
{
	Super::BeginPlay();

	Server_AddInitialSlots();
}

void AChest::Server_AddInitialSlots_Implementation()
{
	for (int i = 0; i < NumberOfSlots; i++)
	{
		Slots.Add(FInventorySlot());
	}
	UpdateInventory();
}
