// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "WeaponComponent.generated.h"

class AGameplayCharacter;

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class CFTWO_API UWeaponComponent : public UActorComponent
{
	GENERATED_BODY()
private:
	// -1 Means no weapon equipped, so character will punch
	int m_CurrentWeapon = -1;

	bool m_CanHit = true;

	// Time that we need to wait to fire OnHit() again, to make sure we will not duplicate its call.
	float CurrentHitInterval = 0.25f;

public:
	AGameplayCharacter* CharacterRef = nullptr;

public:
	// Sets default values for this component's properties
	UWeaponComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:
	// Only called by server to handle collision
	void OnHit();
	void Punch();

	// Validate whether or not we can hit and set m_CanHit.
	bool CanHit();
	void SetCanHit();

	void OnPunch();

};