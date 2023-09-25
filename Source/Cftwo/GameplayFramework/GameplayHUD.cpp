// Fill out your copyright notice in the Description page of Project Settings.


#include "GameplayHUD.h"
#include "../Utils/GeneralFunctionLibrary.h"
#include "../UI/Inventory/InventoryWidget.h"

void AGameplayHUD::BeginPlay()
{
    Super::BeginPlay();
    InventoryWidget = CreateWidget<UInventoryWidget>(GetWorld(), InventoryWidgetClass);
    InventoryWidget->AddToViewport();
}

void AGameplayHUD::UpdateInventory(TArray<FInventorySlot> SlotsRef)
{
    if (!IsValid(InventoryWidget)) {
        MyLog("InventoryWidget == nullptr");
        return;
    }
    
    InventoryWidget->Slots.Empty();
    InventoryWidget->Slots.Append(SlotsRef);
    InventoryWidget->UpdateSlots();
}
