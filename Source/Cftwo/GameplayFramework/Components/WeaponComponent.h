// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "WeaponComponent.generated.h"

class AGameplayCharacter;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class CFTWO_API UWeaponComponent : public UActorComponent
{
	GENERATED_BODY()
private:
	// -1 Means no weapon equipped, so character will punch
	int m_CurrentWeapon = -1;
	AGameplayCharacter* m_CharacterRef = nullptr;
	bool m_CanHit = true;

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
	void OnPunch();
		
};
