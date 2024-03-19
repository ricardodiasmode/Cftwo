// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "InventoryWidget.generated.h"

struct FWeaponInventorySlot;
/**
 * 
 */
UCLASS()
class CFTWO_API UInventoryWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	UPROPERTY(BlueprintReadOnly)
	TArray<struct FInventorySlot> Slots;
	
	UPROPERTY(BlueprintReadOnly)
	TArray<FWeaponInventorySlot> WeaponSlots;

public:
	UFUNCTION(BlueprintImplementableEvent)
	void UpdateSlots();
	
	UFUNCTION(BlueprintCallable)
	void DropItem(const int SlotIndex, const bool WeaponSlot);
	
	UFUNCTION(BlueprintImplementableEvent)
	void SetCraftPopOnSlot(const int SlotIndex);
};
