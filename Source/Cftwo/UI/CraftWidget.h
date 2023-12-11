// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "../GameplayFramework/Components/Inventory/InventorySlot.h"
#include "CraftWidget.generated.h"

/**
 * 
 */
UCLASS()
class CFTWO_API UCraftWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
	void OnUpdateAvailableItems(const TArray<FInventorySlot>& AvailableItems);
	
};
