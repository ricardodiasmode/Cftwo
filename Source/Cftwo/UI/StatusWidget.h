// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "StatusWidget.generated.h"

/**
 * 
 */
UCLASS()
class CFTWO_API UStatusWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	UPROPERTY(BlueprintReadOnly)
	float CurrentHealth = 0.f;
	UPROPERTY(BlueprintReadOnly)
	float MaxHealth = 0.f;

	UPROPERTY(BlueprintReadOnly)
	float CurrentHungry = 0.f;
	UPROPERTY(BlueprintReadOnly)
	float MaxHungry = 0.f;

public:
	UFUNCTION(BlueprintImplementableEvent)
	void OnUpdateHealth();

	UFUNCTION(BlueprintImplementableEvent)
	void OnUpdateHungry();

};
