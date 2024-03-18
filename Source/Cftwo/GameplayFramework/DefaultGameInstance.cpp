// Fill out your copyright notice in the Description page of Project Settings.


#include "DefaultGameInstance.h"
#include "../Misc/DefaultSaveGame.h"
#include "Cftwo/Utils/GeneralFunctionLibrary.h"
#include "Kismet/GameplayStatics.h"

void UDefaultGameInstance::Init()
{
	Super::Init();

	// Retrieve and cast the USaveGame object to UMySaveGame.
	if (UDefaultSaveGame* LoadedGame = Cast<UDefaultSaveGame>(UGameplayStatics::LoadGameFromSlot(SaveSlotName, 0)))
	{
		// The operation was successful, so LoadedGame now contains the data we saved earlier.
		GMyLog("Save game loaded with success.");
		WaveExtentionActivated = LoadedGame->WaveExtentionActivated;
	}
}

void UDefaultGameInstance::SaveGameDelegateFunction(const FString& SlotName, const int32 UserIndex, bool bSuccess)
{
	// useless delegate
}

void UDefaultGameInstance::ActivateWaveExtention()
{
	if (UDefaultSaveGame* SaveGameInstance = Cast<UDefaultSaveGame>(UGameplayStatics::CreateSaveGameObject(UDefaultSaveGame::StaticClass())))
	{
		FAsyncSaveGameToSlotDelegate SavedDelegate;
		// USomeUObjectClass::SaveGameDelegateFunction is a void function that takes the following parameters: const FString& SlotName, const int32 UserIndex, bool bSuccess
		SavedDelegate.BindUObject(this, &UDefaultGameInstance::SaveGameDelegateFunction);

		SaveGameInstance->WaveExtentionActivated = true;
		UGameplayStatics::AsyncSaveGameToSlot(SaveGameInstance, SaveSlotName, 0, SavedDelegate);
	}
}

