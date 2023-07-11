// Fill out your copyright notice in the Description page of Project Settings.


#include "GameplayHUD.h"
#include "../Utils/GeneralFunctionLibrary.h"

void AGameplayHUD::BeginPlay()
{
    InventoryWidget = Cast<UInventoryWidget>(CreateWidget<UInventoryWidget>(GetWorld(), InventoryWidgetClass));
    InventoryWidget->AddToViewport();
}

void AGameplayHUD::UpdateInventory(const TArray<FInventorySlot>& SlotsRef)
{
    PrintDebug("Estamos bem!");
    InventoryWidget->Slots = SlotsRef;
    InventoryWidget->UpdateSlots();
}
