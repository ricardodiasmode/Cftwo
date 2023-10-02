// Fill out your copyright notice in the Description page of Project Settings.


#include "WeaponComponent.h"
#include "../Characters/GameplayCharacter.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "../Actors/BreakableFoliage.h"
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

bool UWeaponComponent::CanHit()
{
	if (!m_CanHit)
		return false;

	static constexpr auto UNUSED_LOOP_TIME = 0.1f;

	m_CanHit = false;
	FTimerHandle UnusedTimer;
	CharacterRef->GetWorldTimerManager().SetTimer(UnusedTimer, this,
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
		OnPunch();
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
	if (UKismetSystemLibrary::CapsuleTraceMultiForObjects(
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
		for (FHitResult CurrentHit : OutHits) {
			if (AGameplayCharacter* CurrentCharacter = Cast<AGameplayCharacter>(CurrentHit.GetActor()))
			{
				// TODO: Damage CurrentCharacter	
			}
			else {
				// Get object display name to know if is a breakable obj
				FString ObjectName = UKismetSystemLibrary::GetDisplayName(CurrentHit.GetActor());

				// Check if was already converted
				PrintDebug("a");
				if (Cast<ABreakableObject>(CurrentHit.GetActor())) {
					ABreakableObject* BreakableObject = Cast<ABreakableObject>(CurrentHit.GetActor());
					BreakableObject->RemoveHP();
					CharacterRef->InventoryComponent->GiveItem(BreakableObject->ItemToGive, 1);
					PrintDebug("b");
				}
				else if (Cast<UInstancedStaticMeshComponent>(CurrentHit.GetComponent()) != nullptr) {
					FString ComponentName = UKismetSystemLibrary::GetDisplayName(CurrentHit.GetComponent());
					if (!ComponentName.Contains("Breakable"))
						return;
					UInstancedStaticMeshComponent* InstancedComp = Cast<UInstancedStaticMeshComponent>(CurrentHit.GetComponent());
					int InstanceIndex = CurrentHit.ElementIndex; // o index ta errado por algum motivo!!!!!!

					PrintDebug("c");
					// Removing foliage
					UStaticMesh* FoliageInstanceMesh = InstancedComp->GetStaticMesh();
					FTransform FoliageInstanceTransform;
					InstancedComp->GetInstanceTransform(InstanceIndex,
						FoliageInstanceTransform, true);
					if (!InstancedComp->RemoveInstance(InstanceIndex))
					{
						PrintDebug("wtf could not remove instance");
						return;
					}
					else {
						PrintDebug("instance removed");
					}

					// Spawning breakable obj
					ABreakableObject* BreakableSpawned = GetWorld()->SpawnActor<ABreakableObject>(ABreakableObject::StaticClass(), FoliageInstanceTransform);
					BreakableSpawned->StaticMeshComponent->SetStaticMesh(FoliageInstanceMesh);

					if (ComponentName.Contains("Rock")) {
						PrintDebug("d");
						BreakableSpawned->ItemToGive = 0;
						int RockIndex = 0;
						BreakableSpawned->ItemToGive = RockIndex;
						BreakableSpawned->RemoveHP();
						CharacterRef->InventoryComponent->GiveItem(RockIndex, 1);
					}
					else if (ComponentName.Contains("Tree")) {
						PrintDebug("e");
						int TreeIndex = 1;
						BreakableSpawned->ItemToGive = TreeIndex;
						CharacterRef->InventoryComponent->GiveItem(TreeIndex, 1);
					}
				}
				else {
					PrintDebugWithVar("%s", *UKismetSystemLibrary::GetDisplayName(CurrentHit.GetComponent()));
				}
			}
		}
	}
}