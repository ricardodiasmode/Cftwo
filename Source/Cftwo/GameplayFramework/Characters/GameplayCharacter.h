// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "../../CftwoCharacter.h"
#include "../Components/Inventory/InventoryComponent.h"
#include "../Components/WeaponComponent.h"
#include "GameplayCharacter.generated.h"

class UCameraComponent;
class USpringArmComponent;

UCLASS()
class CFTWO_API AGameplayCharacter : public ACftwoCharacter
{
	GENERATED_BODY()

private:
	// Controlls the time that player will stop hit after the hit starts.
	// Note that this time may follow anim hit time
	static constexpr auto TIME_TO_STOP_HITTING = 0.35f;

	// Handle OnStopHitting function
	FTimerHandle HitTimerHandle;

	/** Hit Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
		class UInputAction* HitAction;

		/** Craft Input Action */
		UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
		class UInputAction* CraftAction;

		/** Change Item Input Action */
		UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
		class UInputAction* ChangeItemAction;

		/** Pick Item Input Action */
		UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
		class UInputAction* PickItemAction;

public:
	// Controlls whether or not player is hitting
	UPROPERTY(BlueprintReadOnly, Replicated)
		bool Hitting = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UWeaponComponent* WeaponComponent = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInventoryComponent* InventoryComponent = nullptr;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = Weapon, meta = (AllowPrivateAccess = "true"))
	UStaticMeshComponent* WeaponMesh = nullptr;

	UPROPERTY(Replicated)
	float CurrentHealth = 100.f;

	float MaxHealth = 100.f;

	UPROPERTY(Replicated)
	float CurrentHungry = 100.f;

	float MaxHungry = 100.f;

private:
	// Trigger player hitting status and set timer for stop hitting
	UFUNCTION(Server, Reliable, BlueprintCallable)
	void Server_OnHit();
	// Actually check hit collision
	UFUNCTION(BlueprintCallable, Server, Reliable)
	void Server_TriggerHitDamage();
	// Set flag telling that character needs to stop hitting
	void OnStopHitting();
	
	// Server check whether or not player can craft the selected item
	UFUNCTION(Server, Reliable)
	void Server_TryCraft(const int ItemIndex);
	void OnCraft();

	void OnChangeItem(const FInputActionValue& Value);

	UFUNCTION(Server, Reliable)
	void Server_ChangeItem(const bool Forward);

	void Die();

	UFUNCTION(Client, reliable)
	void Client_OnDie();

	// Redirector for input
	void PickItem();

	UFUNCTION(Server, reliable)
	void Server_TryPickItem();

	UFUNCTION(BlueprintCallable)
	void UseItem(const int InventoryIndex);

	UFUNCTION(Server, reliable)
	void Server_TryUseItem(const int InventoryIndex);

protected:

	/** Called for movement input */
	void Move(const FInputActionValue& Value) override;

public:
	// Sets default values for this character's properties
	AGameplayCharacter();

	// Handle for server
	void OnHit();

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	// Handle for server
	UFUNCTION(Client, Reliable, BlueprintCallable)
	void Client_OnCraft(const int ItemIndex);

	UFUNCTION(Client, reliable)
	void Client_InitializeInventory();

	UFUNCTION(Client, reliable)
	void Client_InitializeStatusWidget();

	// Called by server to fire a punch effect
	UFUNCTION(BlueprintCallable)
	void OnPunch();

	int GetEquippedWeaponItemId();
	int GetEquippedWeaponId();

	bool IsEquippedWeaponFireWeapon();

	UFUNCTION(BlueprintPure)
	bool FireWeaponEquipped() { return WeaponComponent->FireWeaponEquipped; }

	FWeaponItem GetWeaponInfo(const int WeaponId) { return InventoryComponent->GetWeaponInfo(WeaponId); }

	int GetWeaponIdOnSlot(const int Id);

	UFUNCTION(Server, reliable)
	void Server_OnGetHitted(const float Damage);

	void OnWeaponChange(UStaticMesh* WeaponMeshRef);

	void AddItem(TPair<int, int> ItemToAdd);
};
