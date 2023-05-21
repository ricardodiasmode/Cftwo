// Fill out your copyright notice in the Description page of Project Settings.


#include "WeaponComponent.h"
#include "../Characters/GameplayCharacter.h"
#include "../../GeneralFunctionLibrary.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "../Actors/BreakableFoliage.h"

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
//void UWeaponComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
//{
//	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
//
//	// ...
//}


void UWeaponComponent::OnHit()
{
	if (m_CurrentWeapon == -1)
		Punch();
}


void UWeaponComponent::Punch()
{
	static constexpr auto TRACE_DIST_FROM_CHARACTER = 50.f;
	static constexpr auto TRACE_RADIUS = 50.f;
	static constexpr auto TRACE_HALF_HEIGHT= 90.f;

	FVector ForwardVector = m_CharacterRef->GetActorForwardVector();
	FVector InitialLocation = m_CharacterRef->GetActorLocation();
	FVector TraceStartLocation = InitialLocation + ForwardVector * TRACE_DIST_FROM_CHARACTER;
	TArray<AActor*> ActorsToIgnore;
	FHitResult OutHit;

	// Check if collides with something
	UKismetSystemLibrary::CapsuleTraceSingle(GetWorld(), TraceStartLocation,
		TraceStartLocation, TRACE_RADIUS, TRACE_HALF_HEIGHT, ETraceTypeQuery::TraceTypeQuery1, false, ActorsToIgnore,
		EDrawDebugTrace::ForDuration, OutHit, true);

	// Check if the collided object was a foliage
	if (Cast<UInstancedStaticMeshComponent>(OutHit.GetComponent()))
	{
		FString ComponentName = UKismetSystemLibrary::GetDisplayName(OutHit.GetComponent());
		if (ComponentName.Contains("BreakableRock"))
		{
			UStaticMesh* MeshRef = Cast<UStaticMeshComponent>(OutHit.GetComponent())->GetStaticMesh();
			FTransform MeshTransform = OutHit.GetComponent()->GetComponentTransform();
			ABreakableFoliage* SpawnedBreakableRef = GetWorld()->SpawnActor<ABreakableFoliage>(ABreakableFoliage::StaticClass(), MeshTransform);
			SpawnedBreakableRef->m_StaticMesh->SetStaticMesh(MeshRef);

			Cast<UInstancedStaticMeshComponent>(OutHit.GetComponent())->RemoveInstance(OutHit.ElementIndex);
		}
	}
	else if (ABreakableFoliage* BreakableFoliage = Cast<ABreakableFoliage>(OutHit.GetActor()))
	{
		if (BreakableFoliage->GetHitted())
		{
			BreakableFoliage->Destroy();
			// TODO: Gives character the item
		}
	}
}

