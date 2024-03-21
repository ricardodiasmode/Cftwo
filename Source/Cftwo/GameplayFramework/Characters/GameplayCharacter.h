// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "../Components/Inventory/InventoryComponent.h"
#include "../Components/WeaponComponent.h"
#include "Cftwo/Actors/Chest.h"
#include "Cftwo/Actors/SpawnableActor.h"
#include "GameFramework/Character.h"
#include "GameplayCharacter.generated.h"

struct FActorMatrix;
struct FIntPair;
class AGameplayGameState;
class AWorkbench;
struct FInputActionValue;
class UCameraComponent;
class USpringArmComponent;
class USphereComponent;

UCLASS()
class CFTWO_API AGameplayCharacter : public ACharacter, public SpawnableActor
{
	GENERATED_BODY()
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class USkeletalMeshComponent* Helmet = nullptr;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class USkeletalMeshComponent* Chest = nullptr;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class USkeletalMeshComponent* Pants = nullptr;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class USkeletalMeshComponent* Shoes = nullptr;
	
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
	static constexpr auto TIME_TO_STOP_HITTING = .6f;

	// Handle OnStopHitting function
	FTimerHandle HitTimerHandle;

	FTimerHandle LockAimTimerHandle;

	FTimerHandle HungryTimerHandle;

	/** Hit Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
		class UInputAction* HitAction;

		/** Pick Item Input Action */
		UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
		class UInputAction* PickItemAction;

	int CurrentDefensePoints = 0;

	AGameplayHUD* HUDRef = nullptr;

	bool Invulnerable = false;

	TArray<FActorMatrix> ActorsSpawned;

protected:

	UPROPERTY(EditDefaultsOnly)
	TObjectPtr<USoundBase> SoundToFireWhenHitSuccess;

	UPROPERTY(EditDefaultsOnly)
	TObjectPtr<USoundBase> SoundToFireWhenPlaceItem;

	UPROPERTY(EditDefaultsOnly)
	TObjectPtr<USoundBase> SoundToFireWhenHitted;

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UCameraShakeBase> DefaultShakeClass;

	UPROPERTY(EditDefaultsOnly)
	float SpawnAngle = 10.f;

	UPROPERTY(EditDefaultsOnly)
	float SpawnDistance = 10000.f;

	UPROPERTY(EditDefaultsOnly)
	float TimeBetweenSpawnActors = 3.f;

	UPROPERTY(EditDefaultsOnly)
	float DespawnDistance = 20000.f;

	UPROPERTY(BlueprintReadWrite)
	class USkeletalMeshComponent* Beckpack = nullptr;
	
public:

	UPROPERTY(BlueprintReadOnly)
	bool IsAdAvailableToContinue = true;

	UPROPERTY(BlueprintReadWrite)
	USphereComponent* WorldCollision = nullptr;

	UPROPERTY(BlueprintReadOnly)
	TArray<ABreakableObject*> BreakableRocks;

	UPROPERTY(BlueprintReadOnly)
	bool HasHammer = false;

	UPROPERTY(BlueprintReadOnly)
	TArray<ABreakableObject*> BreakableTrees;

	UPROPERTY(BlueprintReadOnly)
	TArray<APickable*> Pickables;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class UStaticMeshComponent* LeftHandItemComponent = nullptr;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class UStaticMeshComponent* RightHandItemComponent = nullptr;
	
	// Controlls whether or not player is hitting
	UPROPERTY(BlueprintReadWrite, Replicated)
		bool Hitting = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UWeaponComponent* WeaponComponent = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInventoryComponent* InventoryComponent = nullptr;

	UPROPERTY(ReplicatedUsing=OnRep_CurrentHealth)
	float CurrentHealth = 100.f;

	float MaxHealth = 100.f;

	UPROPERTY(ReplicatedUsing=OnRep_CurrentHungry)
	float CurrentHungry = 100.f;

	float MaxHungry = 100.f;
	
	class AActorSpawner* SpawnerRef = nullptr;
	class AGameplayCharacter* CharacterSpawnerRef = nullptr;

	TArray<AActor*> CloseInteractable;
	
	UPROPERTY(EditDefaultsOnly)
	UNiagaraSystem* DustVFX = nullptr;
	
	UPROPERTY(EditDefaultsOnly)
	UNiagaraSystem* BloodVFX = nullptr;

	UPROPERTY(BlueprintReadOnly, Replicated)
	bool Dead = false;

	// <id, amount>
	UPROPERTY(Replicated)
	TArray<FIntPair> CurrentBuildings;
	
	UPROPERTY(BlueprintReadOnly)
	TObjectPtr<AGameplayGameState> GameState = nullptr;

	UPROPERTY(EditAnywhere)
	TArray<struct FActorToSpawn> ActorsToSpawn;
	
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

	void Die();

	UFUNCTION(Client, reliable)
	void Client_OnDie();

	// Redirector for input
	void PickItem();

	UFUNCTION(Server, reliable)
	void Server_TryPickItem();

	UFUNCTION(BlueprintCallable)
	void UseItem(const int InventoryIndex);

	UFUNCTION(BlueprintCallable)
	void ConvertItem(const int InventoryIndex);

	UFUNCTION(Server, reliable)
	void Server_TryUseItem(const int InventoryIndex);

	UFUNCTION(Server, reliable)
	void Server_TryConvertItem(const int InventoryIndex);

	UFUNCTION(NetMulticast, reliable)
	void Multicast_ChangeEquipment(USkeletalMeshComponent* SKMRef, USkeletalMesh* MeshToSet);

	bool SpawnActor(int Index);
	void CheckShouldSpawnActors();
	
protected:
	UFUNCTION()
	virtual void OnRep_CurrentHealth();
	
	UFUNCTION()
	virtual void OnRep_CurrentHungry();

	/** Called for movement input */
	void Move(const FInputActionValue& Value);

	/** Called for looking input */
	void Look(const FInputActionValue& Value);

	// APawn interface
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	virtual void Destroyed() override;

	virtual void BeginPlay() override;
	
	void SpawnAllActors();

	UFUNCTION(Client, reliable)
	void Client_OnCharacterGetCloseToChest(AChest* ChestRef);

	UFUNCTION(Client, reliable)
	void Client_OnCharacterGetFarToChest(AChest* ChestRef);
	
	UFUNCTION()
	void OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void OnComponentEndOverlap(class UPrimitiveComponent* OverlappedComp, class AActor* OtherActor, class UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);
	
public:
	// Sets default values for this character's properties
	AGameplayCharacter();

	UFUNCTION(Server, reliable)
	void Server_StartSpawningActors();

	UFUNCTION(BlueprintCallable)
	void GiveItem(const int Index, const int Amount) { InventoryComponent->GiveItem(Index, Amount); }

	UFUNCTION(Client, reliable)
	void Client_OnHitWithoutRightWeapon();

	UFUNCTION(BlueprintCallable)
	void OnOverlapInteractable(AActor* OtherActor);

	UFUNCTION(BlueprintCallable)
	void OnEndOverlapInteractable(AActor* OtherActor);

	UFUNCTION(BlueprintCallable)
	void SetRotationAccordingToVelocity(const float DeltaTime);

	UFUNCTION(Client, reliable)
	void Client_OnSetPlayerController();

	UFUNCTION(Server, reliable)
	void Server_OnSetPlayerController();

	/** Returns CameraBoom subobject **/
	FORCEINLINE class USpringArmComponent* GetCameraBoom() const { return CameraBoom; }
	/** Returns FollowCamera subobject **/
	FORCEINLINE class UCameraComponent* GetFollowCamera() const { return FollowCamera; }

	// Handle for server
	void OnHit();
	
	// Handle for server
	UFUNCTION(Client, Reliable, BlueprintCallable)
	void Client_OnCraft(const int ItemIndex);

	void InitializeInventory() const;

	void InitializeStatusWidget();

	void UpdateHungryWidget();
	void HungryAndHealthTick();
	void AddHungry(int Amount);

	void OnPunch();

	UFUNCTION(BlueprintPure)
	int GetEquippedWeaponId() const;

	UFUNCTION(BlueprintPure)
	bool WeaponOnRightHand() const;

	bool IsEquippedWeaponFireWeapon() const;

	UFUNCTION(BlueprintPure)
	bool FireWeaponEquipped() const { return IsEquippedWeaponFireWeapon(); }

	FWeaponItem GetWeaponInfo(const int WeaponId) const { return InventoryComponent->GetWeaponInfo(WeaponId); }

	int GetWeaponIdOnSlot(const int Id);
	void UpdateHealthWidget();
	void RemoveHealth(int Amount);
	void AddHealth(const int Amount);

	UFUNCTION(Server, reliable)
	void Server_OnGetHitted(const float Damage);

	UFUNCTION(Client, reliable)
	void Client_ShakeCamera();

	UFUNCTION(Client, reliable, BlueprintCallable)
	void Client_PlaySound(USoundBase* SoundToPlay);

	UFUNCTION(Client, reliable)
	void Client_OnGetHitted();

	void AddOrDropItem(TPair<int, int> ItemToAdd);

	void OnLoseActor(AActor* ActorRef);
	
	void OnDie();

	FVector GetLockPoint() const { return LockPoint->GetComponentLocation(); }

	void EquipItemOnIndex(const int InventoryIndex);

	UFUNCTION(BlueprintCallable)
	void DropItem(const int SlotIndex);

	UFUNCTION(BlueprintCallable)
	void Pickup();
	
	void OnUpdateInventory(TArray<FInventorySlot> Slots, const TArray<FWeaponInventorySlot>& WeaponSlots);

	void SpawnItemOnHand(FInventoryItem ItemInfo, const int Amount);

	UFUNCTION(BlueprintPure)
	bool CheckCanConvertItem(const int InventoryIndex) const;

	UFUNCTION(BlueprintCallable)
	void SwapSlots(const int FirstSlotIndex, const int SecondSlotIndex);

	UFUNCTION(BlueprintPure)
	bool HasItem(const int ItemIndex) const;

	UFUNCTION(BlueprintPure)
	int GetItemOnIndex(const int SlotIndex);
	
	void SpawnPlaceableAhead(TSubclassOf<AActor> Class) const;

	UFUNCTION(Server, Reliable, BlueprintCallable)
	void Server_SwapChestSlots(AChest* ChestRef, const int FirstChestIndex, const int SecondChestIndex);

	UFUNCTION(Server, reliable, BlueprintCallable)
	void Server_SwapChestInventorySlots(AChest* ChestRef, const int ChestIndex, const int InventoryIndex, const bool FromChest);
	
	UFUNCTION(Server, reliable, BlueprintCallable)
	void Server_DropChestSlot(AChest* ChestRef, const int ChestIndex);

	UFUNCTION(Server, reliable)
	void Server_OnHitSuccess();

	UFUNCTION(Client, reliable)
	void Client_OnHitSuccess();

	UFUNCTION(BlueprintImplementableEvent)
	void BP_OnDie();
	
	void OnTryBuyBuilding(const int BuildingIndex);

	UFUNCTION(Server, reliable)
	void Server_OnTryBuyBuilding(const int BuildingIndex);
	
	UFUNCTION(Client, reliable)
	void Client_OnBuyBuilding(const int BuildingIndex);
	
	UFUNCTION(Client, reliable)
	void Client_OnUseBuilding(const int BuildingIndex);
	
	void RemoveBuilding(const int BuildingIndex);

	UFUNCTION(Server, reliable)
	void Server_RemoveBuilding(const int BuildingIndex);

	void OnContinue();

	UFUNCTION(BlueprintImplementableEvent)
	void OnGetHammer();
	
	void CheckShouldDespawnActors();
};
