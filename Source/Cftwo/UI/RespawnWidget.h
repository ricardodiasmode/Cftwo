// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "RespawnWidget.generated.h"

/**
 * 
 */
UCLASS()
class CFTWO_API URespawnWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	UFUNCTION(BlueprintCallable)
	void OnRespawn();
};
