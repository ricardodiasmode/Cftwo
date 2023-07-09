// Fill out your copyright notice in the Description page of Project Settings.
#include "BreakableObject.h"
#include "../Utils/GeneralFunctionLibrary.h"

// Sets default values
ABreakableObject::ABreakableObject()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	StaticMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("UStaticMeshComponent"));
	RootComponent = StaticMeshComponent;
	StaticMeshComponent->SetCollisionObjectType(ECC_WorldStatic);
}

// Called when the game starts or when spawned
void ABreakableObject::BeginPlay()
{
	Super::BeginPlay();
	
}

void ABreakableObject::RemoveHP()
{
	CurrentHP--;
	PrintDebug("Removing breakable object HP");
	if (CurrentHP < 1)
	{
		Break();	
	} else {
		StaticMeshComponent->SetWorldScale3D(StaticMeshComponent->GetComponentScale()*0.9f);
	}
}	

void ABreakableObject::Break()
{
	// Break effects
	Destroy();
}	

