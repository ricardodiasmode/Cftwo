// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Cftwo/Actors/BreakableObject.h"
#include "Components/ActorComponent.h"
#include "Engine/DataTable.h"
#include "WeaponComponent.generated.h"

class UNiagaraComponent;
class UNiagaraSystem;
class ABaseProjectile;

USTRUCT(BlueprintType)
struct FFireWeaponItem : public FTableRowBase
{
	GENERATED_USTRUCT_BODY()
public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int Index = -1;
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FString Name = "None";
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	UStaticMesh* Mesh = nullptr;
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FString Socket = "";
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FTransform RelativeTransform;
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TSubclassOf<ABaseProjectile> Projectile;
};

class AGameplayCharacter;

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class CFTWO_API UWeaponComponent : public UActorComponent
{
	GENERATED_BODY()
private:
	static constexpr int PUNCH_DAMAGE = 30.f;

	bool m_CanHit = true;

	// Time that we need to wait to fire OnHit() again, to make sure we will not duplicate its call.
	float CurrentHitInterval = 0.25f;

public:
	AGameplayCharacter* CharacterRef = nullptr;

private:
	void TryFireWeapon();

	UFUNCTION(Server, reliable)
	void Server_SpawnProjectile(FVector CameraForward, FVector CameraLoc, TSubclassOf<ABaseProjectile> ProjectileToSpawnClass);

	UFUNCTION(Client, reliable)
	void Client_SpawnProjectile(TSubclassOf<ABaseProjectile> ProjectileToSpawnClass);

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:
	// Sets default values for this component's properties
	UWeaponComponent();

	int GetEquippedWeapon();
	
	// Only called by server to handle collision
	void OnHit();

	UNiagaraComponent* SpawnVFXAtLocation(UNiagaraSystem* VFXToSpawn, const FVector& LocationToSpawn, const FRotator& RotationToSpawn);
	
	void SpawnVFXOnAttack(FHitResult CurrentHit,  AActor* Target, UNiagaraSystem* VFX);
	
	void OnPunch();

	// Validate whether or not we can hit and set m_CanHit.
	bool CanHit();
	void SetCanHit();


	int GetEquippedWeaponByIndex(const int Id);

	void TryLockAim();
};