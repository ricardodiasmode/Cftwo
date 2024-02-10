// Fill out your copyright notice in the Description page of Project Settings.


#include "BaseNeutralCharacter.h"

#include "AIController.h"
#include "../../Actors/ActorSpawner.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Cftwo/GameplayFramework/Characters/GameplayCharacter.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Net/UnrealNetwork.h"

// Sets default values
ABaseNeutralCharacter::ABaseNeutralCharacter()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;
	PrimaryActorTick.bStartWithTickEnabled = false;

	LockPoint = CreateDefaultSubobject<USceneComponent>(TEXT("LockPoint"));
	LockPoint->SetupAttachment(GetMesh());
}

void ABaseNeutralCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const 
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(ABaseNeutralCharacter, CurrentHealth);
}

// Called to bind functionality to input
void ABaseNeutralCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

void ABaseNeutralCharacter::Server_OnGetHitted_Implementation(const float Damage, AActor* Agressor)
{
	if (!GetController())
		return;
	if (!Cast<AAIController>(GetController()))
		return;
	if (!Cast<AAIController>(GetController())->GetBlackboardComponent())
		return;
		
	CurrentHealth -= Damage;

	Cast<AAIController>(GetController())->GetBlackboardComponent()->SetValueAsObject(AgressorBlackboardName, Agressor);
	
	if (CurrentHealth <= 0)
		Die();
}

TArray<TPair<int, int>> ABaseNeutralCharacter::OnHarvest()
{
	TArray<TPair<int, int>> ReturnArray;
	while (ReturnArray.Num() == 0 && ItemsToDrop.Num() > 0) // Make sure we will add something
	{
		int i = 0;
		for (auto& Elem : ItemsToDrop)
		{
			i++;
			const int RandomChance = FMath::RandRange(1, 99);
			if (RandomChance > Elem.Value.First)
				continue;
		
			const float RandomAmount = FMath::RandRange(1, ItemsToDrop[Elem.Key].Second);

			ReturnArray.Add({Elem.Key, RandomAmount});
		
			HarvestLeft--;
			if (HarvestLeft == 0)
			{
				Destroy();
				break;
			}
		}
	}
	return ReturnArray;
}

void ABaseNeutralCharacter::Server_OnDie_Implementation()
{
	if(!SpawnerRef)
	{
		GPrintDebug("Something is wrong, spawner not setted on neutral character.");
		return;
	}
	SpawnerRef->OnLoseActor(this);	
}

void ABaseNeutralCharacter::Server_TriggerHit_Implementation()
{
	UGameplayStatics::SpawnSoundAtLocation(GetWorld(),
		SoundToPlayWhenHit,
		GetActorLocation());
	
	FVector START_LOCATION = GetActorLocation() +
		GetActorForwardVector() * 125.f + GetActorUpVector() * 30.f;
	FVector END_LOCATION = START_LOCATION;
	float RADIUS = 60.f;
	float HALF_HEIGHT = 60.f;
	TArray<TEnumAsByte<EObjectTypeQuery>> ObjTypes;
	ObjTypes.Add(UEngineTypes::ConvertToObjectType(ECC_Pawn));
	TArray<AActor*> ActorsToIgnore;
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
		EDrawDebugTrace::Persistent,
		OutHits,
		true
	))
	{
		for (FHitResult CurrentHit : OutHits)
		{
			// If target is character
			if (AGameplayCharacter* CurrentCharacter = Cast<AGameplayCharacter>(CurrentHit.GetActor()))
			{
				CurrentCharacter->Server_OnGetHitted(DamageToDeal);
				// SpawnVFXOnAttack(CurrentHit, CurrentCharacter, CharacterRef->BloodVFX);
				// SpawnSFXOnAttack(CurrentHit);
				return;
			}
		}
	}
}

void ABaseNeutralCharacter::Destroyed()
{
	Server_OnDie();
	
	Super::Destroyed();
}
