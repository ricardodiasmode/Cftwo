// Copyright Epic Games, Inc. All Rights Reserved.

#include "CftwoGameMode.h"
#include "CftwoCharacter.h"
#include "UObject/ConstructorHelpers.h"

ACftwoGameMode::ACftwoGameMode()
{
	// set default pawn class to our Blueprinted character
	/*static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/ThirdPerson/Blueprints/BP_ThirdPersonCharacter"));
	if (PlayerPawnBPClass.Class != NULL)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
	}*/
}
