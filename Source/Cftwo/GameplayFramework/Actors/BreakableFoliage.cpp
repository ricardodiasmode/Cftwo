// Fill out your copyright notice in the Description page of Project Settings.


#include "BreakableFoliage.h"
#include "Components/StaticMeshComponent.h"
#include "../../GeneralFunctionLibrary.h"

// Sets default values
ABreakableFoliage::ABreakableFoliage()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	m_StaticMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StaticMeshComponent"));
	RootComponent = m_StaticMesh;

	bReplicates = true;
}

// Called when the game starts or when spawned
void ABreakableFoliage::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
bool ABreakableFoliage::GetHitted()
{
	m_CurrentHealth -= 1;
	PrintDebugWithVar("Health after getting hitted: %d", m_CurrentHealth);
	FVector NewScale = m_StaticMesh->GetRelativeScale3D() * 0.75f;
	m_StaticMesh->SetRelativeScale3D(NewScale);

	return m_CurrentHealth == 0;
}
