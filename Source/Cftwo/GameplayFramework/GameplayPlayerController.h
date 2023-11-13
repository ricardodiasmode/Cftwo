// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "GameplayPlayerController.generated.h"

/**
 * 
 */
UCLASS()
class CFTWO_API AGameplayPlayerController : public APlayerController
{
	GENERATED_BODY()

protected:
	virtual void BeginPlay() override;
	
public:
	UFUNCTION(Server, reliable)
	void Server_AskToRespawn();
};
