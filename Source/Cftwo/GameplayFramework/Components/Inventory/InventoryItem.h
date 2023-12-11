// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Engine/DataTable.h"
#include "CoreMinimal.h"
#include "ItemRecipe.h"
#include "../../../Actors/BaseProjectile.h"
#include "InventoryItem.generated.h"

UENUM(BlueprintType)
enum class EItemType : uint8 {
	NONE = 0 UMETA(DisplayName = "NONE"),
	FOOD = 1  UMETA(DisplayName = "FOOD"),
	WEAPON = 2     UMETA(DisplayName = "WEAPON"),
	HEAL = 3     UMETA(DisplayName = "HEAL"),
};

USTRUCT(BlueprintType)
struct FInventoryItem : public FTableRowBase
{
	GENERATED_USTRUCT_BODY()
public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int Index = -1;
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FString Name = "None";
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	UTexture2D* Icon = nullptr;
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int MaxStack = 1;
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TArray<FItemRecipe> Recipe;
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int WeaponId = -1;
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	UStaticMesh* Mesh = nullptr;
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int BuffOnUse = 0.f;
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	EItemType ItemType = EItemType::NONE;
};

USTRUCT(BlueprintType)
struct FWeaponItem : public FTableRowBase
{
	GENERATED_USTRUCT_BODY()
public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int Index = -1;
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	bool FireWeapon = false;
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TSubclassOf<ABaseProjectile> ProjectileClassToSpawn;
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	UStaticMesh* Mesh = nullptr;
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FTransform AttachTransform;
};
