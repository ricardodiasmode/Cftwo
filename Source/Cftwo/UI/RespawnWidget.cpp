// Fill out your copyright notice in the Description page of Project Settings.


#include "RespawnWidget.h"
#include "../GameplayFramework/GameplayHUD.h"
#include "Cftwo/GameplayFramework/Characters/GameplayCharacter.h"

void URespawnWidget::OnRespawn()
{
	APlayerController* ControllerRef = Cast<APlayerController>(GetOwningPlayer());
	if (ControllerRef) {
		AGameplayHUD* HUDRef = Cast<AGameplayHUD>(ControllerRef->GetHUD());
		GetOwningPlayerPawn()->GetWorldTimerManager().ClearAllTimersForObject(GetOwningPlayerPawn());
		HUDRef->OnRespawn();
	}
}

void URespawnWidget::OnContinue()
{
	APlayerController* ControllerRef = Cast<APlayerController>(GetOwningPlayer());
	if (ControllerRef) {
		AGameplayHUD* HUDRef = Cast<AGameplayHUD>(ControllerRef->GetHUD());
		HUDRef->OnContinue();
	}
}

