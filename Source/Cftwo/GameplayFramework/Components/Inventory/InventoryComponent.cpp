// Fill out your copyright notice in the Description page of Project Settings.


#include "InventoryComponent.h"
#include "Math/UnrealMathUtility.h"
#include "Net/UnrealNetwork.h"
#include "../../../Actors/Pickable.h"
#include "../../GameplayHUD.h"
#include "../../../Utils/GeneralFunctionLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "../../Characters/GameplayCharacter.h"

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

void UInventoryComponent::BeginPlay()
{
	Super::BeginPlay();

	for (int i=0;i<MAX_INVENTORY_SIZE;i++)
		Slots.Add(FInventorySlot());
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

		Slots[i] = SlotToAdd;

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
		ItemMap.Add(ItemIndex, AddedAmount);
		Client_UpdateInventory(Slots);
		return true;
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

FInventoryItem UInventoryComponent::GetItemInfo(const int Index)
{
	return *(ItemsDataTable->FindRow<FInventoryItem>(FName(*(FString::FromInt(Index))), ""));
}

FWeaponItem UInventoryComponent::GetWeaponInfo(const int Index)
{
	return *(WeaponsDataTable->FindRow<FWeaponItem>(FName(*(FString::FromInt(Index))), ""));
}

FEquipmentItem UInventoryComponent::GetEquipmentInfo(const int Index)
{
	return *(EquipmentDataTable->FindRow<FEquipmentItem>(FName(*(FString::FromInt(Index))), ""));
}

bool UInventoryComponent::HasItemsToCraft(const int ItemToCraft, TArray<int>* Indexes, TArray<int>* Amount)
{
	FInventoryItem ItemInfo = GetItemInfo(ItemToCraft);
	for (FItemRecipe CurrentRecipe : ItemInfo.Recipe)
	{
		bool Found = false;
		HasRecipe(CurrentRecipe, &Found, Indexes, Amount);

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
			const int InitialAmount = LocalAmount;
			LocalAmount = FMath::Max(LocalAmount - CurrentSlot.Amount, 0);
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
		Slots[SlotIndex] = FInventorySlot();
	else
		Slots[SlotIndex].Amount -= Amount;

	Client_UpdateInventory(Slots);
}

void UInventoryComponent::TryCraft(const int ItemToCraft)
{
	// Check whether or not has all necessary items
	TArray<int> Indexes;
	TArray<int> Amount;
	bool CanCraft = HasItemsToCraft(ItemToCraft, &Indexes, &Amount);

	if (!CanCraft)
		return;

	// If has necessary items, then remove them and create the new one
	for (int i = 0; i < Indexes.Num(); i++) {
		RemoveItem(Indexes[i], Amount[i]);
	}

	GiveItem(ItemToCraft, 1);
}

void UInventoryComponent::DropAllItems()
{
	FActorSpawnParameters SpawnInfo;
	FVector LocationToSpawn = GetOwner()->GetActorLocation() + GetOwner()->GetActorForwardVector() * 50.f;

	for (int i = 0; i < Slots.Num(); i++)
	{
		if (Slots[i].Amount == 0)
			continue;

		const int ItemIndex = Slots[i].ItemInfo.Index;
		const int ItemAmount = Slots[i].Amount;
		FTransform TransformToSpawn(FTransform(FRotator(0), LocationToSpawn, FVector(1)));
		APickable* CurrentPickable = GetWorld()->SpawnActorDeferred<APickable>(PickableClass, TransformToSpawn, GetOwner(), Cast<APawn>(GetOwner()), ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn);
		CurrentPickable->ItemId = ItemIndex;
		CurrentPickable->Amount = ItemAmount;
		UGameplayStatics::FinishSpawningActor(CurrentPickable, TransformToSpawn);
	}
}

bool UInventoryComponent::UseItem(const int InventoryIndex)
{
	const EItemType CurrentItemType = Slots[InventoryIndex].ItemInfo.ItemType;
	if (CurrentItemType == EItemType::FOOD)
	{
		AGameplayCharacter* CharacterRef = Cast<AGameplayCharacter>(GetOwner());
		const float AmountToSet = FMath::Clamp(CharacterRef->CurrentHungry + Slots[InventoryIndex].ItemInfo.BuffOnUse,
			0.f, CharacterRef->MaxHungry);
		CharacterRef->CurrentHungry = AmountToSet;
		RemoveItem(InventoryIndex, 1);
		return true;
	}
	
	if (CurrentItemType == EItemType::HEAL)
	{
		AGameplayCharacter* CharacterRef = Cast<AGameplayCharacter>(GetOwner());
		const float AmountToSet = FMath::Clamp(CharacterRef->CurrentHealth + Slots[InventoryIndex].ItemInfo.BuffOnUse,
			0.f, CharacterRef->MaxHealth);
		CharacterRef->CurrentHealth = AmountToSet;
		RemoveItem(InventoryIndex, 1);
		return true;
	}
	return false;
}

bool UInventoryComponent::ItemOnIndexIsOfType(const int SlotIndex, const EItemType TypeToCheck)
{
	return Slots[SlotIndex].ItemInfo.ItemType == TypeToCheck;
}

FEquipmentItem UInventoryComponent::GetEquipmentInfoFromSlotIndex(const int InventoryIndex)
{
	return GetEquipmentInfo(Slots[InventoryIndex].ItemInfo.OtherDataTableId);
}
