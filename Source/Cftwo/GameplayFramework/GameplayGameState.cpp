// Fill out your copyright notice in the Description page of Project Settings.


#include "GameplayGameState.h"
#include "Net/UnrealNetwork.h"

void AGameplayGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const 
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AGameplayGameState, CurrentWave);
}
