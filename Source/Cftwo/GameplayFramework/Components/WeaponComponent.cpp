// Fill out your copyright notice in the Description page of Project Settings.


#include "WeaponComponent.h"
#include "Kismet/KismetSystemLibrary.h"
#include "../Characters/GameplayCharacter.h"
#include "../../Actors/BreakableObject.h"
#include "../../Utils/GeneralFunctionLibrary.h"
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
void UWeaponComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
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
				} else if(Cast<UInstancedStaticMeshComponent>(CurrentHit.GetComponent()) != nullptr) {
					if (!UKismetSystemLibrary::GetDisplayName(CurrentHit.GetComponent()).Contains("Breakable"))
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
				}
			}
		}
	}
}

