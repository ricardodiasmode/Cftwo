// Fill out your copyright notice in the Description page of Project Settings.


#include "InventoryWidget.h"

#include "Cftwo/GameplayFramework/Characters/GameplayCharacter.h"

void UInventoryWidget::DropItem(const int SlotIndex, const bool WeaponSlot)
{
	if (WeaponSlot)
		Cast<AGameplayCharacter>(GetOwningPlayerPawn())->InventoryComponent->DropItemFromWeaponSlot(SlotIndex);
	else
		Cast<AGameplayCharacter>(GetOwningPlayerPawn())->DropItem(SlotIndex);
}

