// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "Components/Inventory/InventoryComponent.h"
#include "GameplayHUD.generated.h"

class UInventoryWidget;
/**
 * 
 */
UCLASS()
class CFTWO_API AGameplayHUD : public AHUD
{
	GENERATED_BODY()
private:
	UPROPERTY()
	UInventoryWidget* InventoryWidget = nullptr;
	
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UUserWidget> InventoryWidgetClass;

protected:
	virtual void BeginPlay() override;

public:
	void UpdateInventory(TArray<FInventorySlot> SlotsRef);
	
};
