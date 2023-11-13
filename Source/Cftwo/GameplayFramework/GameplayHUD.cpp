// Fill out your copyright notice in the Description page of Project Settings.


#include "GameplayHUD.h"
#include "../Utils/GeneralFunctionLibrary.h"
#include "../UI/Inventory/InventoryWidget.h"
#include "../UI/RespawnWidget.h"
#include "../UI/CraftWidget.h"
#include "../UI/StatusWidget.h"
#include "GameplayPlayerController.h"
#include "Characters/GameplayCharacter.h"

void AGameplayHUD::BeginPlay()
{
    Super::BeginPlay();

    /*InventoryWidget = CreateWidget<UInventoryWidget>(GetWorld(), InventoryWidgetClass);
    InventoryWidget->AddToViewport();

    CraftWidget = CreateWidget<UCraftWidget>(GetWorld(), CraftWidgetClass);
    CraftWidget->AddToViewport();*/
}

void AGameplayHUD::UpdateInventory(TArray<FInventorySlot> SlotsRef)
{
    if (!IsValid(InventoryWidget)) {
        return;
    }

    InventoryWidget->Slots.Empty();
    InventoryWidget->Slots.Append(SlotsRef);
    InventoryWidget->UpdateSlots();

    CraftWidget->OnUpdateAvailableItems(SlotsRef);
}

void AGameplayHUD::InitializeStatusWidget()
{
    AGameplayCharacter* CharacterRef = Cast<AGameplayCharacter>(GetOwningPawn());
    StatusWidget = CreateWidget<UStatusWidget>(GetWorld(), StatusWidgetClass);
    StatusWidget->MaxHealth = CharacterRef->MaxHealth;
    StatusWidget->CurrentHealth = CharacterRef->CurrentHealth;
    StatusWidget->MaxHungry = CharacterRef->MaxHungry;
    StatusWidget->CurrentHungry = CharacterRef->CurrentHungry;
    StatusWidget->AddToViewport();
    StatusWidget->OnUpdateHealth();
    StatusWidget->OnUpdateHungry();
}

void AGameplayHUD::OnRespawn()
{
    GetOwningPlayerController()->SetShowMouseCursor(false);
    GetOwningPlayerController()->SetInputMode(FInputModeGameOnly());
    AGameplayPlayerController* PCRef = Cast<AGameplayPlayerController>(GetOwningPlayerController());
    PCRef->Server_AskToRespawn();
}

void AGameplayHUD::OnUpdateHealth(const float CurrentHealth)
{
    StatusWidget->CurrentHealth = CurrentHealth;
    StatusWidget->OnUpdateHealth();
}

void AGameplayHUD::OnUpdateHungry(const float CurrentHungry)
{
    StatusWidget->CurrentHungry = CurrentHungry;
    StatusWidget->OnUpdateHungry();
}


void AGameplayHUD::OnDie()
{
    GetOwningPlayerController()->SetShowMouseCursor(true);
    GetOwningPlayerController()->SetInputMode(FInputModeUIOnly());
    URespawnWidget* RespawnWidget = CreateWidget<URespawnWidget>(GetWorld(), RespawnWidgetClass);
    RespawnWidget->AddToViewport();
}
