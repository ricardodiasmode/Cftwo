// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "InventoryItem.h"
#include "InventorySlot.h"
#include "InventoryComponent.generated.h"

class AGameplayHUD;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class CFTWO_API UInventoryComponent : public UActorComponent
{
	GENERATED_BODY()
private:
	int MAX_INVENTORY_SIZE = 2;

	// Map of <Index, Amount> where {Index} is the identifier of the item in DT and
	// {Amount} is the amount in the inventory
	TMap<int, int> ItemMap;

public:
	AGameplayHUD* CharacterHUD = nullptr;

	UPROPERTY(EditDefaultsOnly)
	UDataTable* ItemsDataTable = nullptr;

	UPROPERTY(EditDefaultsOnly)
	UDataTable* WeaponsDataTable = nullptr;

	UPROPERTY(EditDefaultsOnly)
	UDataTable* EquipmentDataTable = nullptr;

	UPROPERTY(EditDefaultsOnly)
	UDataTable* BackpackDataTable = nullptr;

	// Array of items in the slot
	UPROPERTY(ReplicatedUsing = UpdateInventory)
	TArray<FInventorySlot> Slots;

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<class APickable> PickableClass;

private:
	/** Try to add the item in the correct slot
	 * @return The amount of the {ItemIndex} that can be added given the amount that want add
	*/
	int CanReceiveItem(int ItemIndex, int Amount, int& FoundSlot);

	/** Tells client to update his inventory HUD */
	UFUNCTION(Client, reliable)
	void Client_UpdateInventory(const TArray<FInventorySlot>& SlotsRef);

	/** Tells client to pop craft effect on slot */
	UFUNCTION(Client, reliable)
	void Client_SetCraftPopOnSlot(const int SlotIndex);

	FInventoryItem GetItemInfo(const int Index) const;

protected:
	virtual void BeginPlay() override;

public:	
	// Sets default values for this component's properties
	UInventoryComponent();

	/** Try to give the desired item to the owning player 
	 * @param ItemIndex: The index of th item to give to the player
	 * @param Amount: The amount to give
	 * @return Where the given item is (the first found slot)
	*/
	int GiveItem(int ItemIndex, int Amount);

	void RemoveItem(const int SlotIndex, const int Amount);

	UFUNCTION()
	void UpdateInventory();

	void TryCraft(const int ItemToCraft);
	
	bool HasItemsToCraft(const int ItemToCraft, TArray<int>* Indexes, TArray<int>* Amount);

	void HasRecipe(FItemRecipe Recipe, bool* Found, TArray<int>* Indexes, TArray<int>* Amount);

	bool IsWeapon(const int ItemId) { return GetItemInfo(ItemId).ItemType == EItemType::WEAPON; }

	bool IsFireWeapon(const int ItemId) { return GetWeaponInfo(ItemId).FireWeapon; }

	FWeaponItem GetWeaponInfo(const int Index) const;
	
	FEquipmentItem GetEquipmentInfo(const int Index) const;
	
	FBackpackItem GetBackpackInfo(const int Index) const;

	void DropAllItems();

	bool UseItem(const int InventoryIndex);

	bool ItemOnIndexIsOfType(const int SlotIndex, const EItemType TypeToCheck);

	FEquipmentItem GetEquipmentInfoFromSlotIndex(const int InventoryIndex);

	void DropItem(const int SlotIndex);
	
	void ConvertItem(const int SlotIndex, const int AmountToRemove, const int AmountToGive);
	
	bool HasItemOnAnyHand(const int ItemIndex);
	bool HasItem(int ItemIndex);

	void SwapSlots(const int FirstSlotIndex, const int SecondSlotIndex);
	
	void IncreaseNumberOfSlots(const int NumberOfSlots);
	
	FBackpackItem GetBackpackInfoFromSlotIndex(const int InventoryIndex);
};
