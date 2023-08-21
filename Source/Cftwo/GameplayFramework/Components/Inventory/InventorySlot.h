// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

USTRUCT(BlueprintType)
struct FInventorySlot
{
	GENERATED_BODY()
public:
	UPROPERTY(BlueprintReadOnly)
		FInventoryItem ItemInfo;
	UPROPERTY(BlueprintReadOnly)
		int Amount = 0;
};
