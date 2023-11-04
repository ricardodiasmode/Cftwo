// Fill out your copyright notice in the Description page of Project Settings.


#include "GameplayHUD.h"
#include "../Utils/GeneralFunctionLibrary.h"
#include "../UI/Inventory/InventoryWidget.h"
#include "../UI/RespawnWidget.h"
#include "GameplayPlayerController.h"

void AGameplayHUD::BeginPlay()
{
    Super::BeginPlay();
    InventoryWidget = CreateWidget<UInventoryWidget>(GetWorld(), InventoryWidgetClass);
    InventoryWidget->AddToViewport();
}

void AGameplayHUD::UpdateInventory(TArray<FInventorySlot> SlotsRef)
{
    if (!IsValid(InventoryWidget)) {
        return;
    }

    InventoryWidget->Slots.Empty();
    InventoryWidget->Slots.Append(SlotsRef);
    InventoryWidget->UpdateSlots();
}

void AGameplayHUD::OnRespawn()
{
    GetOwningPlayerController()->SetShowMouseCursor(false);
    GetOwningPlayerController()->SetInputMode(FInputModeGameOnly());
    AGameplayPlayerController* PCRef = Cast<AGameplayPlayerController>(GetOwningPlayerController());
    PCRef->Server_AskToRespawn();
}

void AGameplayHUD::OnDie()
{
    GetOwningPlayerController()->SetShowMouseCursor(true);
    GetOwningPlayerController()->SetInputMode(FInputModeUIOnly());
    URespawnWidget* RespawnWidget = CreateWidget<URespawnWidget>(GetWorld(), RespawnWidgetClass);
    RespawnWidget->AddToViewport();
}
