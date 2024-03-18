// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "WaveExtentionWidget.generated.h"

/**
 * 
 */
UCLASS()
class CFTWO_API UWaveExtentionWidget : public UUserWidget
{
	GENERATED_BODY()

protected:
	UFUNCTION(BlueprintCallable)
	void OnBuyWaveExtention();
	
};
