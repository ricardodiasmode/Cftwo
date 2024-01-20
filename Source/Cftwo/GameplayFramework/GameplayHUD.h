// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "Components/Inventory/InventoryComponent.h"
#include "../UI/Inventory/InventoryWidget.h"
#include "GameplayHUD.generated.h"

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

	UStatusWidget* StatusWidget = nullptr;

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UUserWidget> InventoryWidgetClass;

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UUserWidget> RespawnWidgetClass;

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UUserWidget> CraftWidgetClass;

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UUserWidget> StatusWidgetClass;
	
protected:
	virtual void BeginPlay() override;

public:
	void UpdateInventory(TArray<FInventorySlot> SlotsRef, const TArray<FWeaponInventorySlot>& WeaponSlotsRef);

	void InitializeStatusWidget(AGameplayCharacter* CharacterRef);

	void OnDie();

	void OnRespawn();

	void OnUpdateHealth(const float CurrentHealth);

	void OnUpdateHungry(const float CurrentHungry);
	
	UFUNCTION(BlueprintImplementableEvent)
	void OnPickableClose();

	UFUNCTION(BlueprintImplementableEvent)
	void OnPickableFar();

	UFUNCTION(BlueprintCallable)
	void OnPickup();
	
	void SetCraftPopOnSlot(const int SlotIndex);

	AGameplayCharacter* CharacterRef = nullptr;
};
