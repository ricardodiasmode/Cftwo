// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "../Components/Inventory/InventoryComponent.h"
#include "../Components/WeaponComponent.h"
#include "Cftwo/Actors/SpawnableActor.h"
#include "GameFramework/Character.h"
#include "GameplayCharacter.generated.h"

struct FInputActionValue;
class UCameraComponent;
class USpringArmComponent;

UCLASS()
class CFTWO_API AGameplayCharacter : public ACharacter, public SpawnableActor
{
	GENERATED_BODY()
	
	/** Where the lock aim will be */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class USceneComponent* LockPoint;
	
	/** Camera boom positioning the camera behind the character */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class USpringArmComponent* CameraBoom;

	/** Follow camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class UCameraComponent* FollowCamera;
	
	/** MappingContext */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	class UInputMappingContext* DefaultMappingContext;

	// /** Jump Input Action */
	// UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	// class UInputAction* JumpAction;

	/** Move Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	class UInputAction* MoveAction;

	/** Look Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	class UInputAction* LookAction;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	float MeshRotationSpeed = 8.f;
	
	// Controlls the time that player will stop hit after the hit starts.
	// Note that this time may follow anim hit time
	static constexpr auto TIME_TO_STOP_HITTING = 0.35f;

	// Handle OnStopHitting function
	FTimerHandle HitTimerHandle;

	FTimerHandle LockAimTimerHandle;

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
	UPROPERTY(BlueprintReadWrite, Replicated)
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
	
	class AActorSpawner* SpawnerRef = nullptr;

private:
	
	// Trigger player hitting status and set timer for stop hitting
	UFUNCTION(Server, Reliable, BlueprintCallable)
	void Server_OnHit(const FRotator& RotationToSet);

	// Check whether or not should lock the aim
	UFUNCTION(Client, Reliable, BlueprintCallable)
	void Client_OnHit(const FRotator& RotationToSet);
	
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
	void Move(const FInputActionValue& Value);

	/** Called for looking input */
	void Look(const FInputActionValue& Value);

	// APawn interface
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	
	// To add mapping context
	virtual void BeginPlay() override;

	virtual void Tick(const float DeltaTime) override;

	virtual void Destroyed() override;

public:
	// Sets default values for this character's properties
	AGameplayCharacter();

	/** Returns CameraBoom subobject **/
	FORCEINLINE class USpringArmComponent* GetCameraBoom() const { return CameraBoom; }
	/** Returns FollowCamera subobject **/
	FORCEINLINE class UCameraComponent* GetFollowCamera() const { return FollowCamera; }

	// Handle for server
	void OnHit();
	
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

	void OnWeaponChange(UStaticMesh* WeaponMeshRef, FTransform WeaponTransform);

	void AddItem(TPair<int, int> ItemToAdd) const;

	void OnDie();

	FVector GetLockPoint() const { return LockPoint->GetComponentLocation(); }

};
