// Fill out your copyright notice in the Description page of Project Settings.


#include "GameplayHUD.h"
#include "../Utils/GeneralFunctionLibrary.h"

void AGameplayHUD::BeginPlay()
{
    InventoryWidget = CreateWidget<UInventoryWidget>(this, InventoryWidgetClass);
    InventoryWidget->AddToViewport();
}

void AGameplayHUD::UpdateInventory(const TArray<FInventorySlot>& SlotsRef)
{
    PrintDebug("Estamos bem!");
}
