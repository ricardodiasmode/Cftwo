// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "GeneralFunctionLibrary.generated.h"

#define GPrintDebug(x) if(GEngine){GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Yellow, TEXT(x));}
#define GPrintDebugWithVar(x, ...) if(GEngine){GEngine->AddOnScreenDebugMessage(-1, 10.0f, FColor::Red, FString::Printf(TEXT(x), __VA_ARGS__));}
#define GMyLog(x) UE_LOG(LogTemp, Warning, TEXT(x));
#define GMyLogWithVar(x, ...) UE_LOG(LogTemp, Warning, TEXT(x), __VA_ARGS__);

USTRUCT()
struct FActorMatrix
{
	GENERATED_BODY()
public:
	TArray<AActor*> ActorArray;

	FActorMatrix() = default;
};

USTRUCT(Blueprintable)
struct FIntPair
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere)
	int First = -1;
	
	UPROPERTY(EditAnywhere)
	int Second = 0;
	
};

/**
 * 
 */
UCLASS()
class CFTWO_API UGeneralFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
	
};
