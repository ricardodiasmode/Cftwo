// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Chest.generated.h"

struct FInventorySlot;

UCLASS()
class CFTWO_API AChest : public AActor
{
	GENERATED_BODY()
protected:
	UPROPERTY(EditDefaultsOnly)
	int NumberOfSlots = 5;
	
public:
	UPROPERTY(ReplicatedUsing = UpdateInventory, BlueprintReadOnly)
	TArray<FInventorySlot> Slots;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Sets default values for this actor's properties
	AChest();
	
	UFUNCTION(BlueprintImplementableEvent)
	void SetWidgetVisibility(const bool Visible);

	UFUNCTION(BlueprintImplementableEvent)
	void UpdateInventory();

	UFUNCTION(Server, reliable)
	void Server_AddInitialSlots();
};
