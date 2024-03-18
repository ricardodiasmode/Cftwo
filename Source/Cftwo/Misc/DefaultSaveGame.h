// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "DefaultSaveGame.generated.h"

/**
 * 
 */
UCLASS()
class CFTWO_API UDefaultSaveGame : public USaveGame
{
	GENERATED_BODY()

public:
	bool WaveExtentionActivated = false;
};
