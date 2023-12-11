// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "InventoryItem.h"
#include "InventorySlot.generated.h"

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
