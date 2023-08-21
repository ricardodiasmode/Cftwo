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
	static constexpr auto MAX_INVENTORY_SIZE = 10;

	// Map of <Index, Amount> where {Index} is the identifier of the item in DT and
	// {Amount} is the amount in the inventory
	TMap<int, int> ItemMap;

	// Array of items in the slot
	UPROPERTY(ReplicatedUsing=UpdateInventory)
	TArray<FInventorySlot> Slots;

public:
	AGameplayHUD* CharacterHUD = nullptr;

	UPROPERTY(EditDefaultsOnly)
	UDataTable* ItemsDataTable = nullptr;

private:
	/** Try to add the item in the correct slot
	 * @return The amount of the {ItemIndex} that can be added given the amount that want add
	*/
	int CanReceiveItem(int ItemIndex, int Amount);

	/** Tells client to update his inventory HUD */
	UFUNCTION(Client, reliable)
	void Client_UpdateInventory(const TArray<FInventorySlot>& SlotsRef);

protected:

public:	
	// Sets default values for this component's properties
	UInventoryComponent();

	/** Try to give the desired item to the owning player 
	 * @param ItemIndex: The index of th item to give to the player
	 * @param Amount: The amount to give
	 * @return Whether or not could give the item 
	*/
	bool GiveItem(int ItemIndex, int Amount);

	UFUNCTION()
	void UpdateInventory();

	void TryCraft(const int ItemToCraft);

	TArray<TTuple<int, int>> HasItemsToCraft(const int ItemToCraft);

	TArray<TTuple<int, int>> HasRecipe(FItemRecipe Recipe, bool& Found);

};
