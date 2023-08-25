// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Engine/DataTable.h"
#include "CoreMinimal.h"
#include "ItemRecipe.h"
#include "InventoryItem.generated.h"

USTRUCT(BlueprintType)
struct FInventoryItem : public FTableRowBase
{
  GENERATED_USTRUCT_BODY()
public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
		int Index = -1;
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
		FString Name = "None";
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
		UTexture2D* Icon = nullptr;
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
		int MaxStack = 1;
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
		TArray<FItemRecipe> Recipe;
};
