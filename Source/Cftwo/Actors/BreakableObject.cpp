// Fill out your copyright notice in the Description page of Project Settings.
#include "BreakableObject.h"
#include "ActorSpawner.h"
#include "Cftwo/GameplayFramework/Characters/GameplayCharacter.h"

// Sets default values
ABreakableObject::ABreakableObject()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;
	PrimaryActorTick.bStartWithTickEnabled = false;

	StaticMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("UStaticMeshComponent"));
	RootComponent = StaticMeshComponent;
	StaticMeshComponent->SetCollisionObjectType(ECC_WorldStatic);
	StaticMeshComponent->PrimaryComponentTick.bCanEverTick = false;
	
	for (UActorComponent* CurrentComponent : GetComponents())
	{
		CurrentComponent->SetCanEverAffectNavigation(false);
	}
}

void ABreakableObject::ShakeOnGetHitted()
{
	if (GetWorldTimerManager().IsTimerActive(ShakeStartTimerHandle))
		GetWorldTimerManager().ClearTimer(ShakeStartTimerHandle);
	if (GetWorldTimerManager().IsTimerActive(ShakeFinishTimerHandle))
		GetWorldTimerManager().ClearTimer(ShakeFinishTimerHandle);

	StaticMeshComponent->SetWorldRotation(InitialMeshRotation);

	// Start shake loop
	GetWorldTimerManager().SetTimer(ShakeStartTimerHandle,
		FTimerDelegate::CreateLambda([this] {
			const FRotator RandomRotation(FMath::RandRange(-ShakeIntensity, ShakeIntensity),
				FMath::RandRange(-ShakeIntensity, ShakeIntensity),
				0.f);
			const FRotator InterpedRotation = FMath::RInterpTo(StaticMeshComponent->GetComponentRotation(),
				StaticMeshComponent->GetComponentRotation() + RandomRotation,
				ShakeInterval,
				8.f);
			StaticMeshComponent->SetWorldRotation(InterpedRotation);
		}),
		ShakeInterval,
		true);

	// Finish shake loop after some time
	GetWorldTimerManager().SetTimer(ShakeFinishTimerHandle,
		FTimerDelegate::CreateLambda([this] {
			if (GetWorldTimerManager().IsTimerActive(ShakeStartTimerHandle))
				GetWorldTimerManager().ClearTimer(ShakeStartTimerHandle);
			StaticMeshComponent->SetWorldRotation(InitialMeshRotation);
		}),
		ShakeDuration,
		false);

	
	
}

// Called when the game starts or when spawned
void ABreakableObject::BeginPlay()
{
	Super::BeginPlay();
	
	StaticMeshComponent->SetStaticMesh(StaticMeshRef);
	InitialMeshRotation = StaticMeshComponent->GetComponentRotation();
	StaticMeshComponent->SetCullDistance(MaxDrawDist);

	if (SpawnerRef)
	{
		for (auto [ActorArray] : SpawnerRef->ActorsSpawned)
		{
			if (ActorArray.Num() == 0)
				return;
		
			if (Cast<AGameplayCharacter>(ActorArray[0]))
			{
				for (AActor* CurrentNPC : ActorArray)
				{
					if (AmITree)
						Cast<AGameplayCharacter>(CurrentNPC)->BreakableTrees.Add(this);
					else
						Cast<AGameplayCharacter>(CurrentNPC)->BreakableRocks.Add(this);
				}
			}
		}
	}
}

void ABreakableObject::RemoveHP()
{
	CurrentHP--;
	if (CurrentHP < 1)
	{
		Break();	
	} else {
		// Remove HP effect
		ShakeOnGetHitted();
	}
}	

void ABreakableObject::Break()
{
	// Break effects
	OnDie();

	for (auto [ActorArray] : SpawnerRef->ActorsSpawned)
	{
		if (ActorArray.Num() == 0)
			return;
		
		if (Cast<AGameplayCharacter>(ActorArray[0]))
		{
			for (AActor* CurrentNPC : ActorArray)
			{
				if (AmITree)
					Cast<AGameplayCharacter>(CurrentNPC)->BreakableTrees.Remove(this);
				else
					Cast<AGameplayCharacter>(CurrentNPC)->BreakableRocks.Remove(this);
			}
		}
	}

	Destroy();
}	

void ABreakableObject::OnDie()
{
	SpawnerRef->OnLoseActor(this);	
}

