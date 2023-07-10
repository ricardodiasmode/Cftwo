// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "Components/Inventory/InventoryComponent.h"
#include "GameplayHUD.generated.h"

/**
 * 
 */
UCLASS()
class CFTWO_API AGameplayHUD : public AHUD
{
	GENERATED_BODY()

public:
	void UpdateInventory(const TArray<FInventorySlot>& SlotsRef);
	
};
