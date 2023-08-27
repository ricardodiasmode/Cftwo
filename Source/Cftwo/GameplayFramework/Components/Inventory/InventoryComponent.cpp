// Fill out your copyright notice in the Description page of Project Settings.


#include "InventoryComponent.h"
#include "Math/UnrealMathUtility.h"
#include "Net/UnrealNetwork.h"
#include "../../GameplayHUD.h"
#include "../../../Utils/GeneralFunctionLibrary.h"

// Sets default values for this component's properties
UInventoryComponent::UInventoryComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}

void UInventoryComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const 
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(UInventoryComponent, Slots);
}

int UInventoryComponent::CanReceiveItem(int ItemIndex, int Amount)
{
	// We use this variable to know how much we added
	int LocalAmount = Amount;
		
	FInventoryItem* ItemInfo = ItemsDataTable->FindRow<FInventoryItem>(FName(*(FString::FromInt(ItemIndex))), "");

	TArray<int> FreeSlots;
	if (ItemMap.Num() == 0)
	{
		for (int i = 0;i < MAX_INVENTORY_SIZE; i++)
			FreeSlots.Add(i);
	} else {
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

			if (CurrentSlot.ItemInfo.Index == -1) {
				FreeSlots.Add(i);
			}
		}
	}
	
	// Check if has free slot and the inventory size can handle an addition
	if (Slots.Num() >= MAX_INVENTORY_SIZE) {
		return 0;
	}

	// If free slots == 0 but we are here, then we can add a new slot
	if (FreeSlots.Num() == 0) {
		for (int i=Slots.Num(); i<MAX_INVENTORY_SIZE-Slots.Num();i++)
			FreeSlots.Add(i);
	}

	// Trying to add into another slot
	for (int i : FreeSlots)
	{
		FInventorySlot SlotToAdd;
		SlotToAdd.ItemInfo = *ItemInfo;

		// Getting how much we can add to this slot
		int MaxAmountToAdd = ItemInfo->MaxStack;		
		int AmountToAdd = FMath::Min(MaxAmountToAdd, LocalAmount);

		// Adding as much as we can
		SlotToAdd.Amount = AmountToAdd;

		// Controlling how much we already add
		LocalAmount -= AmountToAdd;

		Slots.Add(SlotToAdd);

		// If could add everything, then we are good to go
		if (LocalAmount == 0) {
			return Amount;
		}
	}
	
	return Amount - LocalAmount;
}

bool UInventoryComponent::GiveItem(int ItemIndex, int Amount)
{
	int AddedAmount = CanReceiveItem(ItemIndex, Amount);
	if(AddedAmount > 0) {
		PrintDebugWithVar("giving item %d", ItemIndex);
		ItemMap.Add(ItemIndex, AddedAmount);
		Client_UpdateInventory(Slots);
		return true;
	} else {
		PrintDebugWithVar("could not receive item %d", ItemIndex);
	}
	return false;
}

void UInventoryComponent::UpdateInventory()
{
	Client_UpdateInventory(Slots);
}

void UInventoryComponent::Client_UpdateInventory_Implementation(const TArray<FInventorySlot>& SlotsRef)
{
	if (CharacterHUD != nullptr) {
		CharacterHUD->UpdateInventory(SlotsRef);
	}
}

bool UInventoryComponent::HasItemsToCraft(const int ItemToCraft, TArray<int>* Indexes, TArray<int>* Amount)
{
	FInventoryItem ItemInfo = *(ItemsDataTable->FindRow<FInventoryItem>(FName(*(FString::FromInt(ItemToCraft))), ""));
	for (FItemRecipe CurrentRecipe : ItemInfo.Recipe)
	{
		bool Found = false;
		PrintDebugWithVar("Tryinna find %d", CurrentRecipe.Index);
		HasRecipe(CurrentRecipe, &Found, Indexes, Amount);
		if (Found)
		{
			PrintDebugWithVar("Found");

		}
		else {
			PrintDebugWithVar("NOT Found");
		}

		if (!Found)
			return false;
	}
	return true;
}


void UInventoryComponent::HasRecipe(FItemRecipe Recipe, bool* Found, TArray<int>* Indexes, TArray<int>* Amount)
{
	// We use this variable to know how much we removed
	int LocalAmount = Recipe.Amount;

	for (int i = 0; i < Slots.Num(); i++)
	{
		FInventorySlot CurrentSlot = Slots[i];
		if (CurrentSlot.ItemInfo.Index == Recipe.Index) {
			int InitialAmount = LocalAmount;
			LocalAmount -= CurrentSlot.Amount;
			if (InitialAmount > LocalAmount)
			{
				Indexes->Add(i);
				Amount->Add(InitialAmount - LocalAmount);
			}
		}

		if (LocalAmount <= 0) {
			*Found = true;
			return;
		}
	}

	*Found = false;
}

void UInventoryComponent::RemoveItem(const int SlotIndex, const int Amount)
{
	if (Slots[SlotIndex].Amount == Amount)
	{
		Slots[SlotIndex] = FInventorySlot();
	}
	else {
		Slots[SlotIndex].Amount -= Amount;
	}

	Client_UpdateInventory(Slots);
}

void UInventoryComponent::TryCraft(const int ItemToCraft)
{
	// Check whether or not has all necessary items
	TArray<int> Indexes;
	TArray<int> Amount;
	bool CanCraft = HasItemsToCraft(ItemToCraft, &Indexes, &Amount);

	PrintDebug("d");
	if (!CanCraft)
		return;
	PrintDebug("e");

	// If has necessary items, then remove them and create the new one
	for (int i = 0; i < Indexes.Num(); i++)
		RemoveItem(Indexes[i], Amount[i]);

	PrintDebug("f");
	GiveItem(ItemToCraft, 1);
}
