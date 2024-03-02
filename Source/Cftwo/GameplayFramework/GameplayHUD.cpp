// Fill out your copyright notice in the Description page of Project Settings.


#include "GameplayHUD.h"
#include "../UI/RespawnWidget.h"
#include "../UI/CraftWidget.h"
#include "../UI/StatusWidget.h"
#include "GameplayPlayerController.h"
#include "Cftwo/Utils/GeneralFunctionLibrary.h"
#include "Characters/GameplayCharacter.h"

void AGameplayHUD::BeginPlay()
{
    Super::BeginPlay();

    InventoryWidget = CreateWidget<UInventoryWidget>(GetWorld(), InventoryWidgetClass);
    InventoryWidget->AddToViewport();

    // CraftWidget = CreateWidget<UCraftWidget>(GetWorld(), CraftWidgetClass);
    // CraftWidget->AddToViewport();

}

void AGameplayHUD::UpdateInventory(TArray<FInventorySlot> SlotsRef, const TArray<FWeaponInventorySlot>& WeaponSlotsRef)
{
    if (!IsValid(InventoryWidget)) {
        return;
    }

    InventoryWidget->Slots.Empty();
    InventoryWidget->Slots.Append(SlotsRef);
    InventoryWidget->WeaponSlots.Empty();
    InventoryWidget->WeaponSlots.Append(WeaponSlotsRef);
    InventoryWidget->UpdateSlots();

    if (CharacterRef->CloseWorkbenches.Num() > 0)
        FarCloseToWorkbench(false);

    // CraftWidget->OnUpdateAvailableItems(SlotsRef);
}

void AGameplayHUD::InitializeStatusWidget(AGameplayCharacter* OwningCharacter)
{
    if (CharacterRef == nullptr)
        CharacterRef = OwningCharacter;
    
    StatusWidget = CreateWidget<UStatusWidget>(GetWorld(), StatusWidgetClass);
    StatusWidget->MaxHealth = OwningCharacter->MaxHealth;
    StatusWidget->CurrentHealth = OwningCharacter->CurrentHealth;
    StatusWidget->MaxHungry = OwningCharacter->MaxHungry;
    StatusWidget->CurrentHungry = OwningCharacter->CurrentHungry;
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

void AGameplayHUD::OnGetHitted()
{
    if (StatusWidget == nullptr)
        return;

    StatusWidget->OnGetHitted();
}

void AGameplayHUD::OnUpdateHealth(const float CurrentHealth)
{
    if (StatusWidget == nullptr)
        return;
    
    StatusWidget->CurrentHealth = CurrentHealth;
    StatusWidget->OnUpdateHealth();
}

void AGameplayHUD::OnUpdateHungry(const float CurrentHungry)
{
    if (StatusWidget == nullptr)
        return;

    StatusWidget->CurrentHungry = CurrentHungry;
    StatusWidget->OnUpdateHungry();
}


void AGameplayHUD::OnDie()
{
    GetOwningPlayerController()->SetShowMouseCursor(true);
    GetOwningPlayerController()->SetInputMode(FInputModeUIOnly());
    StatusWidget->RemoveFromParent();
    InventoryWidget->RemoveFromParent();
    
    URespawnWidget* RespawnWidget = CreateWidget<URespawnWidget>(GetWorld(), RespawnWidgetClass);
    RespawnWidget->AddToViewport();
}

void AGameplayHUD::OnPickup()
{
    if (!CharacterRef)
        CharacterRef = Cast<AGameplayCharacter>(GetOwningPawn());

    check(CharacterRef); // Here character must be valid

    CharacterRef->Pickup();
}

void AGameplayHUD::SetCraftPopOnSlot(const int SlotIndex)
{
    InventoryWidget->SetCraftPopOnSlot(SlotIndex);
}

void AGameplayHUD::OnMistakenWeapon()
{
    UUserWidget* MistakenWeapon = CreateWidget<UUserWidget>(GetWorld(), MistakenWeaponWidgetClass);
    MistakenWeapon->AddToViewport();
}

void AGameplayHUD::FarCloseToWorkbench(const bool Far)
{
    InventoryWidget->FarCloseToWorkbench(Far);
}

void AGameplayHUD::Server_SwapChestInventorySlots(AChest* ChestRef, const int ChestIndex, const int InventoryIndex)
{
    CharacterRef->Server_SwapChestInventorySlots(ChestRef, ChestIndex, InventoryIndex);
}

void AGameplayHUD::DropChestSlot(AChest* ChestRef, const int ChestIndex)
{
    CharacterRef->Server_DropChestSlot(ChestRef, ChestIndex);
}

void AGameplayHUD::OnTryBuyBuilding(const int BuildingIndex)
{
    CharacterRef->OnTryBuyBuilding(BuildingIndex);
}
