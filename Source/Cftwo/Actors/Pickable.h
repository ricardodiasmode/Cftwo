// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "../GameplayFramework/Components/Inventory/InventoryItem.h"
#include "Pickable.generated.h"

class AActorSpawner;

UCLASS()
class CFTWO_API APickable : public AActor
{
	GENERATED_BODY()

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	class UStaticMeshComponent* StaticMesh = nullptr;

	UPROPERTY(EditDefaultsOnly)
	UDataTable* ItemsDataTable = nullptr;
	
public:

	AActorSpawner* SpawnerRef = nullptr;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Meta = (ExposeOnSpawn=true))
	int ItemId = -1;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Meta = (ExposeOnSpawn=true))
	int Amount = 0;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Sets default values for this actor's properties
	APickable();

	FInventoryItem GetItemInfo(const int Index) const;

	void OnPick();

};
