// Fill out your copyright notice in the Description page of Project Settings.


#include "WeaponComponent.h"
#include "Kismet/KismetSystemLibrary.h"
#include "../Characters/GameplayCharacter.h"
#include "../../Actors/BreakableObject.h"
#include "../../Utils/GeneralFunctionLibrary.h"
#include "Inventory/InventoryComponent.h"
#include "InstancedFoliageActor.h"

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

bool UWeaponComponent::CanHit()
{
	if (!m_CanHit)
		return false;

	static constexpr auto UNUSED_LOOP_TIME = 0.1f;

	m_CanHit = false;
	FTimerHandle UnusedTimer;
	m_CharacterRef->GetWorldTimerManager().SetTimer(UnusedTimer, this, 
		&UWeaponComponent::SetCanHit, UNUSED_LOOP_TIME, false,
		CurrentHitInterval - UNUSED_LOOP_TIME);
	return true;
}

void UWeaponComponent::SetCanHit()
{
	m_CanHit = true;
}


void UWeaponComponent::OnHit()
{
	if (!CanHit())
		return;
		

	if (m_CurrentWeapon == -1)
		Punch();
}


void UWeaponComponent::Punch()
{
	static constexpr auto SIZE_TO_REDUZE_PER_HIT = 0.9f;
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
	if (UInstancedStaticMeshComponent* InstancedCompRef = Cast<UInstancedStaticMeshComponent>(OutHit.GetComponent()))
	{
		FString ComponentName = UKismetSystemLibrary::GetDisplayName(OutHit.GetComponent());
		if (ComponentName.Contains("Breakable")) // If marked (named) as breakable
		{
			// Getting the hitted foliage mesh and transform to copy it
			UStaticMesh* MeshRef = Cast<UStaticMeshComponent>(InstancedCompRef)->GetStaticMesh();
			FTransform MeshTransform;
			InstancedCompRef->GetInstanceTransform(OutHit.Item, MeshTransform, true);

			// Spawning actor with the foliage properties
			ABreakableFoliage* SpawnedBreakableRef = GetWorld()->SpawnActor<ABreakableFoliage>(ABreakableFoliage::StaticClass(), MeshTransform);
			SpawnedBreakableRef->m_StaticMesh->SetStaticMesh(MeshRef);
			// Reducing its size
			FVector NewScale3D = SpawnedBreakableRef->m_StaticMesh->GetRelativeScale3D() * SIZE_TO_REDUZE_PER_HIT;
			SpawnedBreakableRef->m_StaticMesh->SetRelativeScale3D(NewScale3D);

			// Removing instance to not duplicate it
			InstancedCompRef->RemoveInstance(OutHit.Item);

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

void UWeaponComponent::OnPunch()
{
	FVector START_LOCATION = CharacterRef->GetActorLocation() + 
		CharacterRef->GetActorForwardVector() * 75.f;
	FVector END_LOCATION = START_LOCATION;
	float RADIUS = 60.f;
	float HALF_HEIGHT = 90.f;
	TArray<TEnumAsByte<EObjectTypeQuery>> ObjTypes;
	ObjTypes.Add(UEngineTypes::ConvertToObjectType(ECC_Pawn));
	ObjTypes.Add(UEngineTypes::ConvertToObjectType(ECC_WorldStatic));
	TArray<AActor*> ActorsToIgnore;
	ActorsToIgnore.Add(CharacterRef);
	TArray<FHitResult> OutHits;
	if(UKismetSystemLibrary::CapsuleTraceMultiForObjects(
		GetWorld(),
		START_LOCATION,
		END_LOCATION,
		RADIUS,
		HALF_HEIGHT,
		ObjTypes,
		false,
		ActorsToIgnore,
		EDrawDebugTrace::ForDuration,
		OutHits,
		true
	)) {
		for(FHitResult CurrentHit : OutHits) {
			if (AGameplayCharacter* CurrentCharacter = Cast<AGameplayCharacter>(CurrentHit.GetActor()))
			{
				// TODO: Damage CurrentCharacter	
			} else {
				// Get object display name to know if is a breakable obj
				FString ObjectName = UKismetSystemLibrary::GetDisplayName(CurrentHit.GetActor());
				
				// Check if was already converted
				if (Cast<ABreakableObject>(CurrentHit.GetActor())) {
					ABreakableObject* BreakableObject = Cast<ABreakableObject>(CurrentHit.GetActor());
					BreakableObject->RemoveHP();
					CharacterRef->InventoryComponent->GiveItem(BreakableObject->ItemToGive, 1);
				} else if(Cast<UInstancedStaticMeshComponent>(CurrentHit.GetComponent()) != nullptr) {
					FString ComponentName = UKismetSystemLibrary::GetDisplayName(CurrentHit.GetComponent());
					if (!ComponentName.Contains("Breakable"))
						return;
					UInstancedStaticMeshComponent* InstancedComp = Cast<UInstancedStaticMeshComponent>(CurrentHit.GetComponent());
					int InstanceIndex = CurrentHit.ElementIndex;

					// Removing foliage
					UStaticMesh* FoliageInstanceMesh = InstancedComp->GetStaticMesh();
					FTransform FoliageInstanceTransform;
					InstancedComp->GetInstanceTransform(InstanceIndex,
						FoliageInstanceTransform, true);
					InstancedComp->RemoveInstance(InstanceIndex);

					// Spawning breakable obj
					ABreakableObject* BreakableSpawned = GetWorld()->SpawnActor<ABreakableObject>(ABreakableObject::StaticClass(), FoliageInstanceTransform);
					BreakableSpawned->StaticMeshComponent->SetStaticMesh(FoliageInstanceMesh);
				
					if (ComponentName.Contains("Rock")) {
						PrintDebug("a");
						int RockIndex = 0;
						BreakableSpawned->ItemToGive = RockIndex;
						CharacterRef->InventoryComponent->GiveItem(RockIndex, 1);
					} else if (ComponentName.Contains("Tree")) {
						PrintDebug("b");
						int TreeIndex = 1;
						BreakableSpawned->ItemToGive = TreeIndex;
						CharacterRef->InventoryComponent->GiveItem(TreeIndex, 1);
					}
				}
			}
		}
	}
}

