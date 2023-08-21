// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

USTRUCT(BlueprintType)
struct FItemRecipe
{
	GENERATED_USTRUCT_BODY()
public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
		int Index = -1;
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
		int Amount = -1;
};
