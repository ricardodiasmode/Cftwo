// Fill out your copyright notice in the Description page of Project Settings.


#include "RespawnWidget.h"
#include "../GameplayFramework/GameplayHUD.h"

void URespawnWidget::OnRespawn()
{
	APlayerController* ControllerRef = Cast<APlayerController>(GetOwningPlayer());
	if (ControllerRef) {
		AGameplayHUD* HUDRef = Cast<AGameplayHUD>(ControllerRef->GetHUD());
		HUDRef->OnRespawn();
	}
}
