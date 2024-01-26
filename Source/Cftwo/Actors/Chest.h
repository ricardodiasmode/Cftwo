// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Chest.generated.h"

UCLASS()
class CFTWO_API AChest : public AActor
{
	GENERATED_BODY()

public:
	// slots
	

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Sets default values for this actor's properties
	AChest();
	
	UFUNCTION(BlueprintImplementableEvent)
	void SetWidgetVisibility(const bool Visible);
};
