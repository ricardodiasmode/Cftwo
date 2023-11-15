// Fill out your copyright notice in the Description page of Project Settings.


#include "GameplayCharacter.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Net/UnrealNetwork.h"
#include "../Components/WeaponComponent.h"
#include "../Components/Inventory/InventoryComponent.h"
#include "../../Utils/GeneralFunctionLibrary.h"
#include "../GameplayHUD.h"
#include "Kismet/KismetSystemLibrary.h"
#include "../../Actors/Pickable.h"

// Sets default values
AGameplayCharacter::AGameplayCharacter()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	WeaponComponent = CreateDefaultSubobject<UWeaponComponent>(TEXT("WeaponComponent"));
	if(WeaponComponent != nullptr)
		WeaponComponent->CharacterRef = this;

	InventoryComponent = CreateDefaultSubobject<UInventoryComponent>(TEXT("InventoryComponent"));

	WeaponMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("WeaponMesh"));
	WeaponMesh->SetupAttachment(GetMesh(), "DEF-hand_L");
}

void AGameplayCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const 
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AGameplayCharacter, Hitting);
}

void AGameplayCharacter::Client_InitializeInventory_Implementation()
{
	APlayerController* ControllerRef = Cast<APlayerController>(GetController());
	if (ControllerRef) {
		InventoryComponent->CharacterHUD = Cast<AGameplayHUD>(ControllerRef->GetHUD());
		InventoryComponent->UpdateInventory();
	}
}

void AGameplayCharacter::Client_InitializeStatusWidget_Implementation()
{
	APlayerController* ControllerRef = Cast<APlayerController>(GetController());
	if (ControllerRef) {
		Cast<AGameplayHUD>(ControllerRef->GetHUD())->InitializeStatusWidget();
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

		//PickingItem
		EnhancedInputComponent->BindAction(PickItemAction, ETriggerEvent::Started,
			this, &AGameplayCharacter::PickItem);
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
	Client_OnCraft(2);
}

void AGameplayCharacter::Client_OnCraft_Implementation(const int ItemIndex)
{
	Server_TryCraft(ItemIndex);
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

int AGameplayCharacter::GetWeaponIdOnSlot(const int Id)
{
	if (InventoryComponent->Slots.Num() <= Id)
	{
		GPrintDebug("InventoryComponent->Slots.Num() <= Id");
		return -1;

	}
	if (Id == -1)
		return -1;

	FInventorySlot Slot = InventoryComponent->Slots[Id];
	FInventoryItem ItemInfo = Slot.ItemInfo;
	return ItemInfo.WeaponId;
}

void AGameplayCharacter::Server_OnGetHitted_Implementation(const float Damage)
{
	CurrentHealth -= Damage;
	if (CurrentHealth < 0.f)
		CurrentHealth = 0.f;

	APlayerController* ControllerRef = Cast<APlayerController>(GetController());
	if (ControllerRef) {
		Cast<AGameplayHUD>(ControllerRef->GetHUD())->OnUpdateHealth(CurrentHealth);
	}

	if (CurrentHealth <= 0.f)
		Die();
}

void AGameplayCharacter::Die()
{
	InventoryComponent->DropAllItems();
	Client_OnDie();
	Destroy();
}

void AGameplayCharacter::Client_OnDie_Implementation()
{
	APlayerController* ControllerRef = Cast<APlayerController>(GetController());
	if (ControllerRef) {
		AGameplayHUD* CharacterHUD = Cast<AGameplayHUD>(ControllerRef->GetHUD());
		CharacterHUD->OnDie();
	}
}

int AGameplayCharacter::GetEquippedWeaponItemId()
{
	const int WeaponSlotId = WeaponComponent->GetCurrentWeapon();
	if (WeaponSlotId == -1)
		return -1;
	return InventoryComponent->Slots[WeaponSlotId].ItemInfo.Index;
}

int AGameplayCharacter::GetEquippedWeaponId()
{
	const int WeaponSlotId = WeaponComponent->GetCurrentWeapon();
	if (WeaponSlotId == -1)
		return -1;
	return InventoryComponent->Slots[WeaponSlotId].ItemInfo.WeaponId;
}

bool AGameplayCharacter::IsEquippedWeaponFireWeapon()
{
	const int WeaponIdOnItemsDT = GetEquippedWeaponItemId();
	const int WeaponIdOnWeaponsDT = GetEquippedWeaponId();

	if (WeaponIdOnItemsDT == -1 ||
		WeaponIdOnWeaponsDT == -1)
		return false;

	return
		InventoryComponent->IsWeapon(WeaponIdOnItemsDT) &&
		InventoryComponent->IsFireWeapon(WeaponIdOnWeaponsDT);
}

void AGameplayCharacter::OnWeaponChange(UStaticMesh* WeaponMeshRef)
{
	WeaponMesh->SetStaticMesh(WeaponMeshRef);
}

void AGameplayCharacter::PickItem()
{
	Server_TryPickItem();
}

void AGameplayCharacter::Server_TryPickItem_Implementation()
{
	TArray<TEnumAsByte<EObjectTypeQuery>> ObjTypes;
	ObjTypes.Add(UEngineTypes::ConvertToObjectType(ECC_GameTraceChannel1));

	TArray<AActor*> IgnoredActors;
	IgnoredActors.Add(this);

	TArray<FHitResult> OutHits;

	FVector StartLoc = GetActorLocation() + GetActorForwardVector() * 50.f;
	FVector FinishLoc = GetActorLocation() + GetActorForwardVector() * 50.f;
	UKismetSystemLibrary::SphereTraceMultiForObjects(
		GetWorld(),
		StartLoc,
		FinishLoc,
		90.f,
		ObjTypes,
		false,
		IgnoredActors,
		EDrawDebugTrace::None,
		OutHits,
		true
	);

	for (FHitResult CurrentHit : OutHits)
	{
		if (APickable* CurrentPickable = Cast<APickable>(CurrentHit.GetActor()))
		{
			InventoryComponent->GiveItem(CurrentPickable->ItemId, CurrentPickable->Amount);
			CurrentPickable->OnPick();
		}
	}
}

void AGameplayCharacter::UseItem(const int InventoryIndex)
{
	Server_TryUseItem(InventoryIndex);
}

void AGameplayCharacter::Server_TryUseItem_Implementation(const int InventoryIndex)
{
	if (!InventoryComponent->ItemOnIndexIsWeapon(InventoryIndex) &&
		InventoryComponent->UseItem(InventoryIndex))
		return;

	WeaponComponent->SetCurrentWeapon(InventoryIndex);
}

void AGameplayCharacter::AddItem(TPair<int, int> ItemToAdd)
{
	InventoryComponent->GiveItem(ItemToAdd.Key, ItemToAdd.Value);
}
