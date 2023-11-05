// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "Components/Inventory/InventoryComponent.h"
#include "GameplayHUD.generated.h"

class UInventoryWidget;
class UCraftWidget;
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

	UCraftWidget* CraftWidget = nullptr;

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UUserWidget> InventoryWidgetClass;

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UUserWidget> RespawnWidgetClass;

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UUserWidget> CraftWidgetClass;
	
protected:
	virtual void BeginPlay() override;

public:
	void UpdateInventory(TArray<FInventorySlot> SlotsRef);

	void OnDie();

	void OnRespawn();
	
};
