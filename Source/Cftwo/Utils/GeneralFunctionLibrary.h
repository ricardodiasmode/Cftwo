// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "GeneralFunctionLibrary.generated.h"

#define PrintDebug(x) if(GEngine){GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Yellow, TEXT(x));}
#define PrintDebugWithVar(x, ...) if(GEngine){GEngine->AddOnScreenDebugMessage(-1, 10.0f, FColor::Red, FString::Printf(TEXT(x), __VA_ARGS__));}
#define MyLog(x) UE_LOG(LogTemp, Warning, TEXT(x));
#define MyLogWithVar(x, ...) UE_LOG(LogTemp, Warning, TEXT(x), __VA_ARGS__);

/**
 * 
 */
UCLASS()
class CFTWO_API UGeneralFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
	
};
