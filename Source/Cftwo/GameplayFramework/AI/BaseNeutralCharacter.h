// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "BaseNeutralCharacter.generated.h"

UCLASS()
class CFTWO_API ABaseNeutralCharacter : public ACharacter
{
	GENERATED_BODY()

protected:
	UPROPERTY(EditDefaultsOnly)
	TMap<int, int> ItemsToDrop;

	UPROPERTY(BlueprintReadOnly)
	float CurrentHealth = 100.f;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Sets default values for this character's properties
	ABaseNeutralCharacter();

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	UFUNCTION(Server, reliable)
	void Server_OnGetHitted(const float Damage);

	UFUNCTION(BlueprintImplementableEvent)
	void Die();
};
