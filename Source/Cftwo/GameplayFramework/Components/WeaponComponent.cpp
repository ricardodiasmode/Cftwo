// Fill out your copyright notice in the Description page of Project Settings.


#include "WeaponComponent.h"
#include "Kismet/KismetSystemLibrary.h"
#include "../Characters/GameplayCharacter.h"

// Sets default values for this component's properties
UWeaponComponent::UWeaponComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}


// Called when the game starts
void UWeaponComponent::BeginPlay()
{
	Super::BeginPlay();

	// ...
	
}


// Called every frame
void UWeaponComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

void UWeaponComponent::OnPunch()
{
	FVector START_LOCATION = CharacterRef->GetActorLocation() + 
		CharacterRef->GetActorForwardVector() * 50.f;
	FVector END_LOCATION = START_LOCATION;
	float RADIUS = 120.f;
	float HALF_HEIGHT = 90.f;
	TArray<TEnumAsByte<EObjectTypeQuery>> ObjTypes;
	ObjTypes.Add(UEngineTypes::ConvertToObjectType(ECC_Pawn));
	UKismetSystemLibrary::CapsuleTraceMultiForObjects(
		GetWorld(),
		START_LOCATION,
		END_LOCATION,
		RADIUS,
		HALF_HEIGHT,


	)
}

