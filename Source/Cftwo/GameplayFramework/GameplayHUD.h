// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "Components/Inventory/InventoryComponent.h"
#include "../UI/Inventory/InventoryWidget.h"
#include "GameplayHUD.generated.h"

class AChest;
class UCraftWidget;
class UStatusWidget;
class AGameplayCharacter;

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

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UUserWidget> StatusWidgetClass;

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UUserWidget> MistakenWeaponWidgetClass;

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UUserWidget> WaveExtentionWidgetClass;

protected:
	UPROPERTY(BlueprintReadOnly)
	UStatusWidget* StatusWidget = nullptr;

public:
	AGameplayCharacter* CharacterRef = nullptr;
	
protected:
	virtual void BeginPlay() override;

public:
	void UpdateInventory(TArray<FInventorySlot> SlotsRef, const TArray<FWeaponInventorySlot>& WeaponSlotsRef);

	void InitializeStatusWidget(AGameplayCharacter* CharacterRef);

	void OnDie();

	void OnRespawn();
	
	void OnGetHitted();

	void OnUpdateHealth(const float CurrentHealth);

	void OnUpdateHungry(const float CurrentHungry);
	
	UFUNCTION(BlueprintImplementableEvent)
	void OnPickableClose();

	UFUNCTION(BlueprintImplementableEvent)
	void OnPickableFar();

	UFUNCTION(BlueprintImplementableEvent)
	void OnHitSuccess();

	UFUNCTION(BlueprintCallable)
	void OnPickup();
	
	void SetCraftPopOnSlot(const int SlotIndex);
	
	void OnMistakenWeapon();

	void FarCloseToWorkbench(const bool Far);

	void Server_SwapChestInventorySlots(AChest* ChestRef, const int ChestIndex, const int InventoryIndex, const bool FromChest);
	
	void DropChestSlot(AChest* ChestRef, const int ChestIndex);

	UFUNCTION(BlueprintCallable)
	void OnTryBuyBuilding(const int BuildingIndex);

	UFUNCTION(BlueprintCallable)
	void RemoveBuilding(int BuildingIndex);

	UFUNCTION(BlueprintImplementableEvent)
	void OnBuyBuilding(const int BuildingIndex);

	UFUNCTION(BlueprintImplementableEvent)
	void OnUseBuilding(const int BuildingIndex);
	
	void OnBuyWaveExtention();
	
	void CreateWaveExtentionWidget();
};
