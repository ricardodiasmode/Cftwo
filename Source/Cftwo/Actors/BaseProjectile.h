// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "BaseProjectile.generated.h"

class UNiagaraComponent;
class UNiagaraSystem;

UCLASS()
class CFTWO_API ABaseProjectile : public AActor
{
	GENERATED_BODY()
	
protected:
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly)
	class USphereComponent* SphereCollision = nullptr;

	UPROPERTY(EditDefaultsOnly)
	float Damage = 30.f;

	UPROPERTY(EditDefaultsOnly)
	TObjectPtr<UNiagaraSystem> Emmiter;

	UNiagaraComponent* EmmiterRef;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	
	virtual void Tick(float DeltaTime) override;

public:
	// Sets default values for this actor's properties
	ABaseProjectile();

	UFUNCTION()
	void OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

};
