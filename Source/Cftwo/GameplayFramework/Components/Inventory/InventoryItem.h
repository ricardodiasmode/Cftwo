// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Engine/DataTable.h"
#include "CoreMinimal.h"
#include "InventoryItem.generated.h"

USTRUCT(BlueprintType)
struct FInventoryItem : public FTableRowBase
{
  GENERATED_USTRUCT_BODY()
public:
	UPROPERTY(EditAnywhere)
		int Index = -1;
	UPROPERTY(EditAnywhere)
		FString Name = "None";
	UPROPERTY(EditAnywhere)
		UTexture2D* Icon = nullptr;
	UPROPERTY(EditAnywhere)
		int MaxStack = 1;
};
