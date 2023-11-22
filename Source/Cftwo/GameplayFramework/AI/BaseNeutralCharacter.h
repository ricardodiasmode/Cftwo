// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Cftwo/Actors/SpawnableActor.h"
#include "GameFramework/Character.h"
#include "BaseNeutralCharacter.generated.h"

UCLASS()
class CFTWO_API ABaseNeutralCharacter : public ACharacter, public SpawnableActor
{
	GENERATED_BODY()

protected:
	// <id, max amount>
	UPROPERTY(EditDefaultsOnly)
	TMap<int, int> ItemsToDrop;

	UPROPERTY(BlueprintReadOnly)
	float CurrentHealth = 100.f;

	int HarvestLeft = 3;

public:
	class AActorSpawner* SpawnerRef = nullptr;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	
	virtual void Destroyed() override;

public:
	// Sets default values for this character's properties
	ABaseNeutralCharacter();

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	UFUNCTION(Server, reliable)
	void Server_OnGetHitted(const float Damage);

	UFUNCTION(BlueprintImplementableEvent)
	void Die();

	bool AmIAlive() const { return CurrentHealth > 0.f; }

	TPair<int, int> OnHarvest();

	void OnDie();
	
};
