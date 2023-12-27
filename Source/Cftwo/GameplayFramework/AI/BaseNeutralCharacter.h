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

	UPROPERTY(BlueprintReadOnly, Replicated)
	float CurrentHealth = 100.f;

	int HarvestLeft = 3;
	
	/** Where the lock aim will be */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class USceneComponent* LockPoint;

public:
	class AActorSpawner* SpawnerRef = nullptr;

protected:
	
	virtual void Destroyed() override;

public:
	// Sets default values for this character's properties
	ABaseNeutralCharacter();

	FVector GetLockPoint() const { return LockPoint->GetComponentLocation(); }

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	UFUNCTION(Server, reliable)
	void Server_OnGetHitted(const float Damage);

	UFUNCTION(BlueprintImplementableEvent)
	void Die();

	bool AmIAlive() const { return CurrentHealth > 0.f; }

	TArray<TPair<int, int>> OnHarvest();

	UFUNCTION(Server, reliable)
	void Server_OnDie();
	
};
