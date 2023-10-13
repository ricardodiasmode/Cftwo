// Fill out your copyright notice in the Description page of Project Settings.


#include "GameplayCharacter.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Net/UnrealNetwork.h"
#include "../Components/WeaponComponent.h"
#include "../Components/Inventory/InventoryComponent.h"
#include "../../Utils/GeneralFunctionLibrary.h"
#include "../GameplayHUD.h"

// Sets default values
AGameplayCharacter::AGameplayCharacter()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	WeaponComponent = CreateDefaultSubobject<UWeaponComponent>(TEXT("WeaponComponent"));
	if(WeaponComponent != nullptr)
		WeaponComponent->CharacterRef = this;

	InventoryComponent = CreateDefaultSubobject<UInventoryComponent>(TEXT("InventoryComponent"));
}

void AGameplayCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const 
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AGameplayCharacter, Hitting);
}

// Called when the game starts or when spawned
void AGameplayCharacter::BeginPlay()
{
	Super::BeginPlay();

	APlayerController* ControllerRef = Cast<APlayerController>(GetController());
	if (ControllerRef) {
		InventoryComponent->CharacterHUD = Cast<AGameplayHUD>(ControllerRef->GetHUD());
		InventoryComponent->UpdateInventory();
	}
}

// Called to bind functionality to input
void AGameplayCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	// Set up action bindings
	if (UEnhancedInputComponent* EnhancedInputComponent = CastChecked<UEnhancedInputComponent>(PlayerInputComponent)) {

		//Hitting
		EnhancedInputComponent->BindAction(HitAction, ETriggerEvent::Completed,
											this, &AGameplayCharacter::OnHit);
		//Crafting
		EnhancedInputComponent->BindAction(CraftAction, ETriggerEvent::Completed,
			this, &AGameplayCharacter::OnCraft);

		//ChangingItem
		EnhancedInputComponent->BindAction(ChangeItemAction, ETriggerEvent::Started,
			this, &AGameplayCharacter::OnChangeItem);
	}
}

void AGameplayCharacter::OnHit()
{
	Server_OnHit();
}

void AGameplayCharacter::OnChangeItem(const FInputActionValue& Value)
{
	float ValueAsFloat = Value.Get<float>();
	Server_ChangeItem(ValueAsFloat > 0.f);
}

void AGameplayCharacter::Server_ChangeItem_Implementation(const bool Forward)
{
	WeaponComponent->ChangeEquippedWeapon(Forward);
}

void AGameplayCharacter::OnCraft()
{
	Client_OnCraft();
}

void AGameplayCharacter::Client_OnCraft_Implementation()
{
	Server_TryCraft(SelectedItemToCraft);
}

void AGameplayCharacter::Server_TryCraft_Implementation(const int ItemIndex)
{
	InventoryComponent->TryCraft(ItemIndex);
}

void AGameplayCharacter::Server_OnHit_Implementation()
{
	Hitting = true;
	// Offset to make sure blend will behave properly
	static constexpr auto BLEND_OFFSET = 0.1f;
	// Needed in order to timer work
	static constexpr auto LOOP_RATE_TIME = 0.01f;
	if (GetWorldTimerManager().IsTimerActive(HitTimerHandle))
		GetWorldTimerManager().ClearTimer(HitTimerHandle);
	GetWorldTimerManager().SetTimer(HitTimerHandle, this,
		&AGameplayCharacter::OnStopHitting, LOOP_RATE_TIME, false,
		TIME_TO_STOP_HITTING - LOOP_RATE_TIME - BLEND_OFFSET);
}

void AGameplayCharacter::OnStopHitting()
{
	Hitting = false;
}

void AGameplayCharacter::Server_TriggerHitDamage_Implementation()
{
	WeaponComponent->OnHit();
}

void AGameplayCharacter::Move(const FInputActionValue& Value)
{
	// Do not move while hitting
	if (Hitting)
		return;

	Super::Move(Value);
}

void AGameplayCharacter::OnPunch()
{
	WeaponComponent->OnPunch();
}
