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
	UPROPERTY(EditDefaultsOnly)
	TObjectPtr<USoundBase> BackgroundSound;
	
private:
	UFUNCTION(Client, reliable)
	void Client_StartBackgroundSound();
	
protected:
	virtual void BeginPlay() override;
	
public:
	UFUNCTION(Server, reliable)
	void Server_AskToRespawn();

	UFUNCTION(Client, reliable, BlueprintCallable)
	void Client_AskToPunch();

	UFUNCTION(Client, reliable)
	void Client_SetPawnInHUD();

	UFUNCTION(Client, reliable)
	void Client_CreateWaveExtentionWidget();
	
	virtual void OnPossess(APawn* InPawn) override;
};
