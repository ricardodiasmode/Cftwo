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
	EQUIP = 4     UMETA(DisplayName = "EQUIP"),
	BACKPACK = 5     UMETA(DisplayName = "BACKPACK"),
	PLACEABLE = 6     UMETA(DisplayName = "PLACEABLE"),
};

UENUM(BlueprintType)
enum class EEquipmentType : uint8 {
	HELMET = 0 UMETA(DisplayName = "HELMET"),
	CHEST = 1  UMETA(DisplayName = "CHEST"),
	PANTS = 2     UMETA(DisplayName = "PANTS"),
	SHOES = 3     UMETA(DisplayName = "SHOES")
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
	int OtherDataTableId = -1;
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	UStaticMesh* Mesh = nullptr;
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FTransform TransformOnHand;
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int BuffOnUse = 0.f;
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	EItemType ItemType = EItemType::NONE;
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int ConvertTo = -1;
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float CullDistance = 10000.f;
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
	TObjectPtr<USoundBase> SoundToPlay;
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float MeleeDamageMultiplier = 1.f;
};

USTRUCT(BlueprintType)
struct FEquipmentItem : public FTableRowBase
{
	GENERATED_USTRUCT_BODY()
public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int Index = -1;
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	EEquipmentType Type;
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	USkeletalMesh* MeshRef = nullptr;
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int DefensePoints = 0;
};

USTRUCT(BlueprintType)
struct FBackpackItem : public FTableRowBase
{
	GENERATED_USTRUCT_BODY()
public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int Index = -1;
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FTransform TransformOnEquip;
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	UStaticMesh* MeshRef = nullptr;
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int Slots = -1;
};

USTRUCT(BlueprintType)
struct FPlaceableItem : public FTableRowBase
{
	GENERATED_USTRUCT_BODY()
public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TSubclassOf<AActor> ClassToSpawn;
};
