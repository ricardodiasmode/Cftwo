// Fill out your copyright notice in the Description page of Project Settings.


#include "WaveExtentionWidget.h"

#include "Cftwo/GameplayFramework/GameplayHUD.h"

void UWaveExtentionWidget::OnBuyWaveExtention()
{
	AGameplayHUD* HUDRef = Cast<AGameplayHUD>(GetOwningPlayer()->GetHUD());
	HUDRef->OnBuyWaveExtention();
}

