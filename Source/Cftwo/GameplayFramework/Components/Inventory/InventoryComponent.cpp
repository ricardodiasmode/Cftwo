// Fill out your copyright notice in the Description page of Project Settings.


#include "InventoryComponent.h"
#include "Math/UnrealMathUtility.h"
#include "../../GameplayHUD.h"

// Sets default values for this component's properties
UInventoryComponent::UInventoryComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}


// Called when the game starts
void UInventoryComponent::BeginPlay()
{
	Super::BeginPlay();

	for (int i=0; i < MAX_INVENTORY_SIZE; i++)
	{
		Slots.Add(FInventorySlot());
	}
	
}

int UInventoryComponent::CanReceiveItem(int ItemIndex, int Amount)
{
	// We use this variable to know how much we added
	int LocalAmount = Amount;
		
	FInventoryItem ItemInfo = FInventoryItem(); // TODO

	TArray<int> FreeSlots;
	if (ItemMap.Contains(ItemIndex))
	{
		// Find the maximum stack of this item
		for (int i=0; i < Slots.Num(); i++)
		{
			FInventorySlot CurrentSlot = Slots[i];
			if (CurrentSlot.ItemInfo.Index == ItemIndex)
			{ // Adding as much as possible to this stack
				// Getting how much we can add to this slot
				int MaxAmountToAdd = CurrentSlot.ItemInfo.MaxStack - CurrentSlot.Amount;
				
				int AmountToAdd = FMath::Min(MaxAmountToAdd, LocalAmount);
					
				// Adding as much as we can
				Slots[i].Amount += AmountToAdd;

				// Controlling how much we already add
				LocalAmount -= AmountToAdd;

				// If could add everything, then we are good to go
				if (LocalAmount == 0)
					return Amount;
			}

			if (CurrentSlot.ItemInfo.Index == -1)
				FreeSlots.Add(i);
		}
	}
	
	// Check if has free slot and the inventory size can handle an addition
	if (FreeSlots.Num() == 0 || Slots.Num() >= MAX_INVENTORY_SIZE)
		return 0;

	// Trying to add into another slot
	for (int i : FreeSlots)
	{
		FInventorySlot SlotToAdd;
		SlotToAdd.ItemInfo = ItemInfo;

		// Getting how much we can add to this slot
		int MaxAmountToAdd = ItemInfo.MaxStack;		
		int AmountToAdd = FMath::Min(MaxAmountToAdd, LocalAmount);

		// Adding as much as we can
		SlotToAdd.Amount = AmountToAdd;

		// Controlling how much we already add
		LocalAmount -= AmountToAdd;

		// If could add everything, then we are good to go
		if (LocalAmount == 0)
			return Amount;
	}
	
	return Amount - LocalAmount;
}

bool UInventoryComponent::GiveItem(int ItemIndex, int Amount)
{
	int AddedAmount = CanReceiveItem(ItemIndex, Amount);
	if(AddedAmount > 0) {
		ItemMap[ItemIndex] += AddedAmount; 
		Client_UpdateInventory(Slots);
		return true;
	}
	return false;
}

void UInventoryComponent::Client_UpdateInventory_Implementation(const TArray<FInventorySlot>& SlotsRef)
{
	CharacterHUD->UpdateInventory(SlotsRef);
}

