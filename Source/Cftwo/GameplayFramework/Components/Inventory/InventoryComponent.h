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
	static constexpr auto MAX_INVENTORY_SIZE = 5;

	// Map of <Index, Amount> where {Index} is the identifier of the item in DT and
	// {Amount} is the amount in the inventory
	TMap<int, int> ItemMap;

public:
	AGameplayHUD* CharacterHUD = nullptr;

	UPROPERTY(EditDefaultsOnly)
	UDataTable* ItemsDataTable = nullptr;

	UPROPERTY(EditDefaultsOnly)
	UDataTable* WeaponsDataTable = nullptr;

	// Array of items in the slot
	UPROPERTY(ReplicatedUsing = UpdateInventory)
	TArray<FInventorySlot> Slots;

private:
	/** Try to add the item in the correct slot
	 * @return The amount of the {ItemIndex} that can be added given the amount that want add
	*/
	int CanReceiveItem(int ItemIndex, int Amount);

	/** Tells client to update his inventory HUD */
	UFUNCTION(Client, reliable)
	void Client_UpdateInventory(const TArray<FInventorySlot>& SlotsRef);

	FInventoryItem GetItemInfo(const int Index);

protected:
	virtual void BeginPlay() override;

public:	
	// Sets default values for this component's properties
	UInventoryComponent();

	/** Try to give the desired item to the owning player 
	 * @param ItemIndex: The index of th item to give to the player
	 * @param Amount: The amount to give
	 * @return Whether or not could give the item 
	*/
	bool GiveItem(int ItemIndex, int Amount);

	void RemoveItem(const int SlotIndex, const int Amount);

	UFUNCTION()
	void UpdateInventory();

	void TryCraft(const int ItemToCraft);
	
	bool HasItemsToCraft(const int ItemToCraft, TArray<int>* Indexes, TArray<int>* Amount);

	void HasRecipe(FItemRecipe Recipe, bool* Found, TArray<int>* Indexes, TArray<int>* Amount);

	bool IsFireWeapon(const int ItemId) { return GetWeaponInfo(ItemId).FireWeapon; }

	FWeaponItem GetWeaponInfo(const int Index);

};
