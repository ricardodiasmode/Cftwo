// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "DefaultGameInstance.generated.h"

/**
 * 
 */
UCLASS()
class CFTWO_API UDefaultGameInstance : public UGameInstance
{
	GENERATED_BODY()

private:
	static constexpr auto SaveSlotName = "WaveExtentionSlot";

public:
	bool WaveExtentionActivated = false;

protected:
	virtual void Init() override;

public:
	void SaveGameDelegateFunction(const FString& SlotName, const int32 UserIndex, bool bSuccess);
	
	void ActivateWaveExtention();
	
};
