// Fill out your copyright notice in the Description page of Project Settings.


#include "InventoryComponent.h"
#include "Math/UnrealMathUtility.h"
#include "Net/UnrealNetwork.h"
#include "../../../Actors/Pickable.h"
#include "../../GameplayHUD.h"
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

int UInventoryComponent::CanReceiveItem(int ItemIndex, int Amount, int& FoundSlot)
{
	FoundSlot = -1;
	
	// We use this variable to know how much we added
	int LocalAmount = Amount;

	const FInventoryItem* ItemInfo = ItemsDataTable->FindRow<FInventoryItem>(FName(*(FString::FromInt(ItemIndex))), "");

	TArray<int> FreeSlots;
	if (ItemMap.Num() == 0)
	{
		for (int i = 0;i < MAX_INVENTORY_SIZE; i++)
			FreeSlots.Add(i);
	} else {
		// Find the maximum stack of this item
		for (int i=0; i < Slots.Num(); i++)
		{
			const FInventorySlot CurrentSlot = Slots[i];
			if (CurrentSlot.ItemInfo.Index == ItemIndex)
			{ // Adding as much as possible to this stack
				// Getting how much we can add to this slot
				const int MaxAmountToAdd = CurrentSlot.ItemInfo.MaxStack - CurrentSlot.Amount;

				const int AmountToAdd = FMath::Min(MaxAmountToAdd, LocalAmount);
					
				// Adding as much as we can
				Slots[i].Amount += AmountToAdd;

				// Controlling how much we already add
				LocalAmount -= AmountToAdd;

				if (FoundSlot == -1) // Setting the first found slot
					FoundSlot = i;

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
	for (const int i : FreeSlots)
	{
		FInventorySlot SlotToAdd;
		SlotToAdd.ItemInfo = *ItemInfo;

		// Getting how much we can add to this slot
		const int MaxAmountToAdd = ItemInfo->MaxStack;
		const int AmountToAdd = FMath::Min(MaxAmountToAdd, LocalAmount);

		// Adding as much as we can
		SlotToAdd.Amount = AmountToAdd;

		// Controlling how much we already add
		LocalAmount -= AmountToAdd;

		Slots[i] = SlotToAdd;

		if (FoundSlot == -1) // Setting the first found slot
			FoundSlot = i;

		// If could add everything, then we are good to go
		if (LocalAmount == 0) {
			return Amount;
		}
	}
	
	return Amount - LocalAmount;
}

int UInventoryComponent::GiveItem(int ItemIndex, int Amount)
{
	int FoundSlot = -1;
	const int AddedAmount = CanReceiveItem(ItemIndex, Amount, FoundSlot);
	if(AddedAmount > 0) {
		ItemMap.Add(ItemIndex, AddedAmount);

		UpdateInventory();
	}
	return FoundSlot;
}

void UInventoryComponent::UpdateInventory()
{
	Cast<AGameplayCharacter>(GetOwner())->OnUpdateInventory(Slots);
	Client_UpdateInventory(Slots);
}

void UInventoryComponent::Client_UpdateInventory_Implementation(const TArray<FInventorySlot>& SlotsRef)
{
	if (CharacterHUD != nullptr) {
		CharacterHUD->UpdateInventory(SlotsRef);
	}
}

void UInventoryComponent::Client_SetCraftPopOnSlot_Implementation(const int SlotIndex)
{
	if (CharacterHUD != nullptr) {
		CharacterHUD->SetCraftPopOnSlot(SlotIndex);
	}
}

FInventoryItem UInventoryComponent::GetItemInfo(const int Index) const
{
	return *(ItemsDataTable->FindRow<FInventoryItem>(FName(*(FString::FromInt(Index))), ""));
}

FWeaponItem UInventoryComponent::GetWeaponInfo(const int Index) const
{
	return *(WeaponsDataTable->FindRow<FWeaponItem>(FName(*(FString::FromInt(Index))), ""));
}

FEquipmentItem UInventoryComponent::GetEquipmentInfo(const int Index) const
{
	return *(EquipmentDataTable->FindRow<FEquipmentItem>(FName(*(FString::FromInt(Index))), ""));
}

FBackpackItem UInventoryComponent::GetBackpackInfo(const int Index) const
{
	return *(BackpackDataTable->FindRow<FBackpackItem>(FName(*(FString::FromInt(Index))), ""));
}

bool UInventoryComponent::HasItemsToCraft(const int ItemToCraft, TArray<int>* Indexes, TArray<int>* Amount)
{
	FInventoryItem ItemInfo = GetItemInfo(ItemToCraft);
	for (const FItemRecipe CurrentRecipe : ItemInfo.Recipe)
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
		const FInventorySlot CurrentSlot = Slots[i];
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

	UpdateInventory();
}

void UInventoryComponent::TryCraft(const int ItemToCraft)
{
	// Check whether or not has all necessary items
	TArray<int> Indexes;
	TArray<int> Amount;
	const bool CanCraft = HasItemsToCraft(ItemToCraft, &Indexes, &Amount);

	if (!CanCraft)
		return;

	// If has necessary items, then remove them and create the new one
	for (int i = 0; i < Indexes.Num(); i++) {
		RemoveItem(Indexes[i], Amount[i]);
	}

	const int CraftedOnSlot = GiveItem(ItemToCraft, 1);
	if(CraftedOnSlot != -1)
	{
		Client_SetCraftPopOnSlot(CraftedOnSlot);
	} else
	{ // If could not add to inventory, spawn it
		FActorSpawnParameters SpawnInfo;
		const FVector LocationToSpawn = GetOwner()->GetActorLocation() + GetOwner()->GetActorForwardVector() * 50.f;
		constexpr int ItemAmount = 1;
		const FTransform TransformToSpawn(FTransform(FRotator(0), LocationToSpawn, FVector(1)));
		APickable* CurrentPickable = GetWorld()->SpawnActorDeferred<APickable>(PickableClass, TransformToSpawn, GetOwner(), Cast<APawn>(GetOwner()), ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn);
		CurrentPickable->ItemId = ItemToCraft;
		CurrentPickable->Amount = ItemAmount;
		UGameplayStatics::FinishSpawningActor(CurrentPickable, TransformToSpawn);
	}
}

void UInventoryComponent::DropAllItems()
{
	for (int i = 0; i < Slots.Num(); i++)
	{
		if (Slots[i].Amount == 0)
			continue;

		DropItem(i);
	}
}

bool UInventoryComponent::UseItem(const int InventoryIndex)
{
	const EItemType CurrentItemType = Slots[InventoryIndex].ItemInfo.ItemType;
	if (CurrentItemType != EItemType::FOOD &&
		CurrentItemType != EItemType::HEAL) return false;
	
	AGameplayCharacter* CharacterRef = Cast<AGameplayCharacter>(GetOwner());

	if (CurrentItemType == EItemType::FOOD)
		CharacterRef->AddHungry(Slots[InventoryIndex].ItemInfo.BuffOnUse);
	else if (CurrentItemType == EItemType::HEAL)
		CharacterRef->AddHealth(Slots[InventoryIndex].ItemInfo.BuffOnUse);
	
	RemoveItem(InventoryIndex, 1);
	return true;
}

bool UInventoryComponent::ItemOnIndexIsOfType(const int SlotIndex, const EItemType TypeToCheck)
{
	return Slots[SlotIndex].ItemInfo.ItemType == TypeToCheck;
}

void UInventoryComponent::DropItem(const int SlotIndex)
{
	FActorSpawnParameters SpawnInfo;
	const FVector LocationToSpawn = GetOwner()->GetActorLocation() + GetOwner()->GetActorForwardVector() * 50.f;
	const int ItemIndex = Slots[SlotIndex].ItemInfo.Index;
	const int ItemAmount = Slots[SlotIndex].Amount;
	const FTransform TransformToSpawn(FTransform(FRotator(0), LocationToSpawn, FVector(1)));
	APickable* CurrentPickable = GetWorld()->SpawnActorDeferred<APickable>(PickableClass, TransformToSpawn, GetOwner(), Cast<APawn>(GetOwner()), ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn);
	CurrentPickable->ItemId = ItemIndex;
	CurrentPickable->Amount = ItemAmount;
	UGameplayStatics::FinishSpawningActor(CurrentPickable, TransformToSpawn);
	RemoveItem(SlotIndex, ItemAmount);
}

void UInventoryComponent::ConvertItem(const int SlotIndex, const int AmountToRemove, const int AmountToGive)
{
	const int ItemIndexToGive = Slots[SlotIndex].ItemInfo.ConvertTo;
	RemoveItem(SlotIndex, AmountToRemove);
	GiveItem(ItemIndexToGive, AmountToGive);
}

bool UInventoryComponent::HasItemOnAnyHand(const int ItemIndex)
{
	return Slots[0].ItemInfo.Index == ItemIndex || Slots[1].ItemInfo.Index == ItemIndex;
}

bool UInventoryComponent::HasItem(const int ItemIndex)
{
	for (const FInventorySlot& CurrentSlot : Slots)
	{
		if (CurrentSlot.ItemInfo.Index == ItemIndex)
			return true;
	}
	return false;
}

void UInventoryComponent::SwapSlots(const int FirstSlotIndex, const int SecondSlotIndex)
{
	const FInventorySlot FirstSlotCopy = Slots[FirstSlotIndex];
	Slots[FirstSlotIndex] = Slots[SecondSlotIndex];
	Slots[SecondSlotIndex] = FirstSlotCopy;
	UpdateInventory();
}

void UInventoryComponent::IncreaseNumberOfSlots(const int NumberOfSlots)
{
	MAX_INVENTORY_SIZE = NumberOfSlots;
	const int CurrentNumberOfSlots = Slots.Num();
	for (int i = CurrentNumberOfSlots; i < MAX_INVENTORY_SIZE; i++)
		Slots.Add(FInventorySlot());
	UpdateInventory();
}

FEquipmentItem UInventoryComponent::GetEquipmentInfoFromSlotIndex(const int InventoryIndex)
{
	return GetEquipmentInfo(Slots[InventoryIndex].ItemInfo.OtherDataTableId);
}

FBackpackItem UInventoryComponent::GetBackpackInfoFromSlotIndex(const int InventoryIndex)
{
	return GetBackpackInfo(Slots[InventoryIndex].ItemInfo.OtherDataTableId);
}	
