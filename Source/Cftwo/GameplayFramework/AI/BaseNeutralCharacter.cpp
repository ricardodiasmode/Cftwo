// Fill out your copyright notice in the Description page of Project Settings.


#include "BaseNeutralCharacter.h"

// Sets default values
ABaseNeutralCharacter::ABaseNeutralCharacter()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void ABaseNeutralCharacter::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called to bind functionality to input
void ABaseNeutralCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

void ABaseNeutralCharacter::Server_OnGetHitted_Implementation(const float Damage)
{
	CurrentHealth -= Damage;
	if (CurrentHealth <= 0)
		Die();
}

