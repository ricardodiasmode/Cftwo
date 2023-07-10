// Fill out your copyright notice in the Description page of Project Settings.


#include "GameplayCharacter.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Net/UnrealNetwork.h"
#include "../Components/WeaponComponent.h"
#include "../Components/Inventory/InventoryComponent.h"
#include "../GameplayHUD.h"

// Sets default values
AGameplayCharacter::AGameplayCharacter()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	m_WeaponComponent = CreateDefaultSubobject<UWeaponComponent>(TEXT("WeaponComponent"));
	if(m_WeaponComponent != nullptr)
		m_WeaponComponent->CharacterRef = this;

	m_InventoryComponent = CreateDefaultSubobject<UInventoryComponent>(TEXT("InventoryComponent"));
	if (m_InventoryComponent)
	{
		if (APlayerController* ControllerRef = Cast<APlayerController>(GetController()))
			m_InventoryComponent->CharacterHUD = Cast<AGameplayHUD>(ControllerRef->GetHUD());
	}
}

void AGameplayCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const 
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AGameplayCharacter, m_Hitting);
}


// Called when the game starts or when spawned
void AGameplayCharacter::BeginPlay()
{
	Super::BeginPlay();
	
}
//
//// Called every frame
//void AGameplayCharacter::Tick(float DeltaTime)
//{
//	Super::Tick(DeltaTime);
//
//}

// Called to bind functionality to input
void AGameplayCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	// Set up action bindings
	if (UEnhancedInputComponent* EnhancedInputComponent = CastChecked<UEnhancedInputComponent>(PlayerInputComponent)) {

		//Jumping
		EnhancedInputComponent->BindAction(HitAction, ETriggerEvent::Triggered,
											this, &AGameplayCharacter::OnHit);
	}
}

void AGameplayCharacter::OnHit()
{
	Server_OnHit();
}

void AGameplayCharacter::Server_OnHit_Implementation()
{
	m_Hitting = true;
	// Offset to make sure blend will behave properly
	static constexpr auto BLEND_OFFSET = 0.1f;
	// Needed in order to timer work
	static constexpr auto LOOP_RATE_TIME = 0.01f;
	if (GetWorldTimerManager().IsTimerActive(m_HitTimerHandle))
		GetWorldTimerManager().ClearTimer(m_HitTimerHandle);
	GetWorldTimerManager().SetTimer(m_HitTimerHandle, this,
		&AGameplayCharacter::OnStopHitting, LOOP_RATE_TIME, false,
		TIME_TO_STOP_HITTING - LOOP_RATE_TIME - BLEND_OFFSET);
}

void AGameplayCharacter::OnStopHitting()
{
	m_Hitting = false;
}

void AGameplayCharacter::Move(const FInputActionValue& Value)
{
	// Do not move while hitting
	if (m_Hitting)
		return;

	Super::Move(Value);
}

void AGameplayCharacter::OnPunch()
{
	m_WeaponComponent->OnPunch();
}
