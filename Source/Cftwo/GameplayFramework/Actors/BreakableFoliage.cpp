// Fill out your copyright notice in the Description page of Project Settings.


#include "BreakableFoliage.h"
#include "Components/StaticMeshComponent.h"
#include "../../Utils/GeneralFunctionLibrary.h"

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
	static constexpr auto SIZE_TO_REDUZE_PER_HIT = 0.9f;

	m_CurrentHealth -= 1;
	FVector NewScale = m_StaticMesh->GetRelativeScale3D() * SIZE_TO_REDUZE_PER_HIT;
	m_StaticMesh->SetRelativeScale3D(NewScale);

	return m_CurrentHealth == 0;
}
