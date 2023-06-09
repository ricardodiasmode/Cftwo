// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "../../GameplayFramework/Components/Inventory/InventoryComponent.h"
#include "Blueprint/UserWidget.h"
#include "InventoryWidget.generated.h"

/**
 * 
 */
UCLASS()
class CFTWO_API UInventoryWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	UPROPERTY(BlueprintReadOnly)
	TArray<FInventorySlot> Slots;
	
public:
	virtual void NativeConstruct() override;

	UFUNCTION(BlueprintImplementableEvent)
	void UpdateSlots();
};
