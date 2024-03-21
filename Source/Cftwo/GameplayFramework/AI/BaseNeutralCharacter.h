// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Cftwo/Actors/SpawnableActor.h"
#include "Cftwo/Utils/GeneralFunctionLibrary.h"
#include "GameFramework/Character.h"
#include "BaseNeutralCharacter.generated.h"

UCLASS()
class CFTWO_API ABaseNeutralCharacter : public ACharacter, public SpawnableActor
{
	GENERATED_BODY()

protected:
	// <id, probability, max amount>
	UPROPERTY(EditDefaultsOnly)
	TMap<int, FIntPair> ItemsToDrop;

	UPROPERTY(EditDefaultsOnly)
	FName AgressorBlackboardName;

	UPROPERTY(EditDefaultsOnly)
	TArray<TSubclassOf<AActor>> DeathActorsToSpawnOnDie;

	UPROPERTY(EditDefaultsOnly)
	float DamageToDeal = 40.f;

	UPROPERTY(EditDefaultsOnly)
	TObjectPtr<USoundBase> SoundToPlayWhenHit;

	UPROPERTY(EditDefaultsOnly)
	TObjectPtr<USoundBase> SoundToPlayWhenTakeDamage;

	UPROPERTY(EditDefaultsOnly)
	TObjectPtr<USoundBase> SoundToPlayWhenDie;

	int HarvestLeft = 3;
	
	/** Where the lock aim will be */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class USceneComponent* LockPoint;

public:
	class AActorSpawner* SpawnerRef = nullptr;
	
	class AGameplayCharacter* CharacterSpawnerRef = nullptr;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Replicated)
	float CurrentHealth = 100.f;

private:	
	void Server_DropDeathActors();

protected:
	virtual void Destroyed() override;

public:
	// Sets default values for this character's properties
	ABaseNeutralCharacter();

	UFUNCTION(BlueprintCallable, Server, reliable)
	void Server_TriggerHit();

	FVector GetLockPoint() const { return LockPoint->GetComponentLocation(); }

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	UFUNCTION(Server, reliable)
	void Server_OnGetHitted(const float Damage, AActor* Agressor);

	UFUNCTION(NetMulticast, reliable)
	void Multicast_SpawnSoundAtLocation(USoundBase* SoundToPlay);

	UFUNCTION(BlueprintImplementableEvent)
	void Die();

	bool AmIAlive() const { return CurrentHealth > 0.f; }

	TArray<TPair<int, int>> OnHarvest();

	UFUNCTION(Server, reliable)
	void Server_OnDie();

	UFUNCTION(BlueprintImplementableEvent)
	void OnDie();
	
};
