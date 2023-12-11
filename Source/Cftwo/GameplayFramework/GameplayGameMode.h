// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "../CftwoGameMode.h"
#include "GameplayGameMode.generated.h"

class AGameplayCharacter;

/**
 * 
 */
UCLASS()
class CFTWO_API AGameplayGameMode : public ACftwoGameMode
{
	GENERATED_BODY()
private:
	TArray<FVector> AllPlayerStartLocation;

protected:
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<AGameplayCharacter> GameplayCharacterClass;

protected:
	virtual void BeginPlay() override;

	virtual void PostLogin(APlayerController* NewPlayer) override;

public:
	void SpawnPlayerCharacter(APlayerController* NewPlayer);
	
};
