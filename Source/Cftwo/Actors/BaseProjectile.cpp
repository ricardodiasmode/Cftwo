// Fill out your copyright notice in the Description page of Project Settings.


#include "BaseProjectile.h"

#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "../GameplayFramework/Characters/GameplayCharacter.h"
#include "../GameplayFramework/AI/BaseNeutralCharacter.h"
#include "Components/SphereComponent.h"
#include "../Utils/GeneralFunctionLibrary.h"
#include "Kismet/KismetSystemLibrary.h"

// Sets default values
ABaseProjectile::ABaseProjectile()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	SphereCollision = CreateDefaultSubobject<USphereComponent>(TEXT("SphereCollision"));
	RootComponent = SphereCollision;
	SphereCollision->OnComponentBeginOverlap.AddDynamic(this, &ABaseProjectile::OnOverlapBegin);

	for (UActorComponent* CurrentComponent : GetComponents())
	{
		CurrentComponent->SetCanEverAffectNavigation(false);
	}
}

// Called when the game starts or when spawned
void ABaseProjectile::BeginPlay()
{
	Super::BeginPlay();

	SetLifeSpan(5.f);

	if (Emmiter)
	{
		EmmiterRef = UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(),
			Emmiter,
			GetActorLocation(),
			GetActorRotation() - FRotator(90.f, 0.f, 0.f));
	}
}

void ABaseProjectile::Tick(const float DeltaTime)
{
	Super::Tick(DeltaTime);	

	if (IsValid(EmmiterRef))
	{
		EmmiterRef->SetWorldLocation(GetActorLocation());
	}
}

void ABaseProjectile::Server_OnOverlapBegin_Implementation(AActor* OtherActor)
{
	if (AGameplayCharacter* CharacterRef = Cast<AGameplayCharacter>(OtherActor))
	{
		CharacterRef->Server_OnGetHitted(Damage);
		Cast<AGameplayCharacter>(GetOwner())->Server_OnHitSuccess();
	}
	else if (ABaseNeutralCharacter* CurrentIA = Cast<ABaseNeutralCharacter>(OtherActor))
	{
		if (CurrentIA->CurrentHealth <= 0)
			return;
		
		CurrentIA->Server_OnGetHitted(Damage, GetOwner());
		Cast<AGameplayCharacter>(GetOwner())->Server_OnHitSuccess();
	}
}

void ABaseProjectile::OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (OtherActor == GetOwner())
		return;

	Server_OnOverlapBegin(OtherActor);

	if (IsValid(EmmiterRef))
	{
		EmmiterRef->Deactivate();
	}
	
	Destroy();
}

