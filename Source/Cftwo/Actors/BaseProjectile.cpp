// Fill out your copyright notice in the Description page of Project Settings.


#include "BaseProjectile.h"
#include "../GameplayFramework/Characters/GameplayCharacter.h"
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
}

// Called when the game starts or when spawned
void ABaseProjectile::BeginPlay()
{
	Super::BeginPlay();
	
}

void ABaseProjectile::OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (OtherActor == GetOwner())
		return;

	GPrintDebugWithVar("%s", *(UKismetSystemLibrary::GetDisplayName(OtherActor)));

	if (AGameplayCharacter* CharacterRef = Cast<AGameplayCharacter>(OtherActor))
	{
		CharacterRef->Server_OnGetHitted(Damage);
		GPrintDebug("hitted player");
	}
	GPrintDebug("hitted something");
	Destroy();
}


