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

public:
	// Controlls whether or not player is hitting
	UPROPERTY(BlueprintReadOnly, Replicated)
		bool Hitting = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UWeaponComponent* WeaponComponent = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInventoryComponent* InventoryComponent = nullptr;

	int SelectedItemToCraft = 2;

private:
	// Trigger player hitting status and set timer for stop hitting
	UFUNCTION(Server, Reliable)
	void Server_OnHit();
	// Handle for server
	void OnHit();
	// Actually check hit collision
	UFUNCTION(BlueprintCallable, Server, Reliable)
	void Server_TriggerHitDamage();
	// Set flag telling that character needs to stop hitting
	void OnStopHitting();
	
	// Server check whether or not player can craft the selected item
	UFUNCTION(Server, Reliable)
	void Server_TryCraft(const int ItemIndex);

	// Handle for server
	UFUNCTION(Client, Reliable)
	void Client_OnCraft();
	void OnCraft();

	void OnChangeItem(const FInputActionValue& Value);

	UFUNCTION(Server, Reliable)
	void Server_ChangeItem(const bool Forward);

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	/** Called for movement input */
	void Move(const FInputActionValue& Value) override;

public:
	// Sets default values for this character's properties
	AGameplayCharacter();

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	// Called by server to fire a punch effect
	UFUNCTION(BlueprintCallable)
	void OnPunch();

	int GetEquippedWeaponId() const { return WeaponComponent->GetCurrentWeapon(); }

	bool IsEquippedWeaponFireWeapon() { return InventoryComponent->IsFireWeapon(GetEquippedWeaponId()); }

	FWeaponItem GetWeaponInfo(const int WeaponId) { return InventoryComponent->GetWeaponInfo(WeaponId); }

	int GetWeaponIdOnSlot(const int Id);
};
