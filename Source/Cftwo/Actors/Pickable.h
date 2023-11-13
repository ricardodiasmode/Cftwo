// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "../GameplayFramework/Components/Inventory/InventoryItem.h"
#include "Pickable.generated.h"

UCLASS()
class CFTWO_API APickable : public AActor
{
	GENERATED_BODY()

protected:
	class UStaticMeshComponent* StaticMesh = nullptr;

	UPROPERTY(EditDefaultsOnly)
	UDataTable* ItemsDataTable = nullptr;
	
public:
	int ItemId = -1;
	int Amount = 0;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Sets default values for this actor's properties
	APickable();

	FInventoryItem GetItemInfo(const int Index);

	void OnPick();

};