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
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/InputComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Controller.h"
#include "GameFramework/SpringArmComponent.h"
#include "../../Actors/ActorSpawner.h"
#include "Animation/AnimInstanceProxy.h"
#include "Cftwo/Actors/Chest.h"
#include "Cftwo/Actors/Workbench.h"
#include "Components/SphereComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"

// Sets default values
AGameplayCharacter::AGameplayCharacter()
{
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_GameTraceChannel1, ECR_Overlap);
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_GameTraceChannel3, ECR_Overlap);
	GetCapsuleComponent()->OnComponentBeginOverlap.AddDynamic(this, &AGameplayCharacter::OnOverlapBegin);
	GetCapsuleComponent()->OnComponentEndOverlap.AddDynamic(this, &AGameplayCharacter::OnComponentEndOverlap);

	// Don't rotate when the controller rotates. Let that just affect the camera.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = true; // Character moves in the direction of input...	
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 500.0f, 0.0f); // ...at this rotation rate

	// Note: For faster iteration times these variables, and many more, can be tweaked in the Character Blueprint
	// instead of recompiling to adjust them
	GetCharacterMovement()->JumpZVelocity = 700.f;
	GetCharacterMovement()->AirControl = 0.35f;
	GetCharacterMovement()->MaxWalkSpeed = 500.f;
	GetCharacterMovement()->MinAnalogWalkSpeed = 20.f;
	GetCharacterMovement()->BrakingDecelerationWalking = 2000.f;

	// Create a camera boom (pulls in towards the player if there is a collision)
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 400.0f; // The camera follows at this distance behind the character	
	CameraBoom->bUsePawnControlRotation = true; // Rotate the arm based on the controller

	// Create a follow camera
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName); // Attach the camera to the end of the boom and let the boom adjust to match the controller orientation
	FollowCamera->bUsePawnControlRotation = false; // Camera does not rotate relative to arm

	// Note: The skeletal mesh and anim blueprint references on the Mesh component (inherited from Character) 
	// are set in the derived blueprint asset named ThirdPersonCharacter (to avoid direct content references in C++)
	
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	Helmet = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Helmet"));
	Helmet->SetupAttachment(GetMesh());
	Helmet->SetLeaderPoseComponent(GetMesh());
	Helmet->SetRenderCustomDepth(true);

	Chest = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Chest"));
	Chest->SetupAttachment(GetMesh());
	Chest->SetLeaderPoseComponent(GetMesh());
	Chest->SetRenderCustomDepth(true);

	Backpack = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Backpack"));
	Backpack->SetupAttachment(GetMesh(), "spine_fk_003");
	Backpack->SetRenderCustomDepth(true);

	Pants = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Pants"));
	Pants->SetupAttachment(GetMesh());
	Pants->SetLeaderPoseComponent(GetMesh());
	Pants->SetRenderCustomDepth(true);

	Shoes = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Shoes"));
	Shoes->SetupAttachment(GetMesh());
	Shoes->SetLeaderPoseComponent(GetMesh());
	Shoes->SetRenderCustomDepth(true);

	WeaponComponent = CreateDefaultSubobject<UWeaponComponent>(TEXT("WeaponComponent"));
	if(WeaponComponent != nullptr)
		WeaponComponent->CharacterRef = this;

	InventoryComponent = CreateDefaultSubobject<UInventoryComponent>(TEXT("InventoryComponent"));

	LockPoint = CreateDefaultSubobject<USceneComponent>(TEXT("LockPoint"));
	LockPoint->SetupAttachment(GetMesh(), "DEF-spine_003");

	LeftHandItemComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("LeftHandItemComponent"));
	LeftHandItemComponent->SetupAttachment(GetMesh(), "socket_item_hand_l");

	RightHandItemComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("RightHandItemComponent"));
	RightHandItemComponent->SetupAttachment(GetMesh(), "socket_item_hand_r");
	
	for (UActorComponent* CurrentComponent : GetComponents())
	{
		CurrentComponent->SetCanEverAffectNavigation(false);
	}
}

void AGameplayCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const 
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AGameplayCharacter, Hitting);
	DOREPLIFETIME(AGameplayCharacter, CurrentHealth);
	DOREPLIFETIME(AGameplayCharacter, CurrentHungry);
}

//////////////////////////////////////////////////////////////////////////
// Input

void AGameplayCharacter::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent)
{
	// Set up action bindings
	if (UEnhancedInputComponent* EnhancedInputComponent = CastChecked<UEnhancedInputComponent>(PlayerInputComponent)) {
		
		//Jumping
		// EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Triggered, this, &ACharacter::Jump);
		// EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);

		//Moving
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AGameplayCharacter::Move);

		//Looking
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &AGameplayCharacter::Look);

		//Hitting
		EnhancedInputComponent->BindAction(HitAction, ETriggerEvent::Completed,
											this, &AGameplayCharacter::OnHit);

		//PickingItem
		EnhancedInputComponent->BindAction(PickItemAction, ETriggerEvent::Started,
			this, &AGameplayCharacter::PickItem);
	}

}

void AGameplayCharacter::Tick(const float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (GetVelocity().Length() > 10.f && !Hitting)
	{
		FVector EndLoc = GetActorLocation() + GetVelocity();
		EndLoc.Z = GetActorLocation().Z;
		FRotator DesiredRotation = UKismetMathLibrary::FindLookAtRotation(GetActorLocation(), EndLoc);
		DesiredRotation.Yaw -= 90.f;
		GetMesh()->SetWorldRotation(DesiredRotation);
	}
}

void AGameplayCharacter::Move(const FInputActionValue& Value)
{
	// Do not move while hitting
	if (Hitting)
		return;
	
	// input is a Vector2D
	const FVector2D MovementVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		// find out which way is forward
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		// get forward vector
		const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
	
		// get right vector 
		const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

		// add movement 
		AddMovementInput(ForwardDirection, MovementVector.Y);
		AddMovementInput(RightDirection, MovementVector.X);
	}
}

void AGameplayCharacter::Look(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D LookAxisVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		// add yaw and pitch input to controller
		AddControllerYawInput(LookAxisVector.X);
		AddControllerPitchInput(LookAxisVector.Y);
	}
}

void AGameplayCharacter::OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (Cast<APickable>(OtherActor) && ClosePickable == nullptr)
	{
		ClosePickable = Cast<APickable>(OtherActor);

		if (HUDRef)
			HUDRef->OnPickableClose();
	} else if (Cast<AWorkbench>(OtherActor) && !CloseWorkbenches.Contains(Cast<AWorkbench>(OtherActor)))
	{
		CloseWorkbenches.Add(Cast<AWorkbench>(OtherActor));
		
		if (HUDRef)
			HUDRef->FarCloseToWorkbench(false);
	} else if (AChest* ChestRef = Cast<AChest>(OtherActor))
	{
		Client_OnCharacterGetCloseToChest(ChestRef);
	}
}

void AGameplayCharacter::OnComponentEndOverlap(class UPrimitiveComponent* OverlappedComp, class AActor* OtherActor, class UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (Cast<APickable>(OtherActor) == ClosePickable && ClosePickable != nullptr)
	{
		ClosePickable = nullptr;
	
		if (HUDRef)
			HUDRef->OnPickableFar();

		APickable* PickableRef = Cast<APickable>(OtherActor);
		CheckOtherPickableClose(PickableRef->SphereCollision->GetScaledSphereRadius(), PickableRef);
	} else if (Cast<AWorkbench>(OtherActor) && CloseWorkbenches.Contains(OtherActor))
	{
		CloseWorkbenches.Remove(Cast<AWorkbench>(OtherActor));
		
		if (HUDRef && CloseWorkbenches.Num() == 0)
			HUDRef->FarCloseToWorkbench(true);
	} else if (AChest* ChestRef = Cast<AChest>(OtherActor))
	{
		Client_OnCharacterGetFarToChest(ChestRef);
	}
}

void AGameplayCharacter::CheckOtherPickableClose(const float Radius, APickable* PickableToIgnore)
{
	FHitResult OutHit;
	TArray<AActor*> ActorsToIgnore;
	TArray<TEnumAsByte<EObjectTypeQuery>> ObjectTypeQuery;
	ObjectTypeQuery.Add(UEngineTypes::ConvertToObjectType(ECC_GameTraceChannel1));
	UKismetSystemLibrary::SphereTraceSingleForObjects(GetWorld(),
		GetActorLocation(),
		GetActorLocation(),
		Radius,
		ObjectTypeQuery,
		false,
		ActorsToIgnore,
		EDrawDebugTrace::None,
		OutHit,
		true);

	if (Cast<APickable>(OutHit.GetActor()) && Cast<APickable>(OutHit.GetActor()) != PickableToIgnore)
	{
		ClosePickable = Cast<APickable>(OutHit.GetActor());

		if (HUDRef)
			HUDRef->OnPickableClose();
	}
}

void AGameplayCharacter::InitializeInventory() const
{
	InventoryComponent->CharacterHUD = HUDRef;
	InventoryComponent->UpdateInventory();
}

void AGameplayCharacter::InitializeStatusWidget()
{
	if (HUDRef)
		HUDRef->InitializeStatusWidget(this);
}

void AGameplayCharacter::Client_OnSetPlayerController_Implementation()
{
	if (const APlayerController* PlayerController = Cast<APlayerController>(Controller))
	{
		//Add Input Mapping Context
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			Subsystem->AddMappingContext(DefaultMappingContext, 0);
		}

		// Caching HUD
		HUDRef = Cast<AGameplayHUD>(PlayerController->GetHUD());
		InitializeInventory();
		InitializeStatusWidget();
	}
}

void AGameplayCharacter::UpdateHungryWidget()
{
	if (HUDRef)
	{
		HUDRef->OnUpdateHungry(CurrentHungry);
	} else if (Cast<APlayerController>(GetController())) {
		if (Cast<APlayerController>(GetController())->GetHUD())
		{
			HUDRef = Cast<AGameplayHUD>(Cast<APlayerController>(GetController())->GetHUD());
			HUDRef->OnUpdateHungry(CurrentHungry);
		}
	}
}

void AGameplayCharacter::RemoveHungry()
{ // must be called only by server
	CurrentHungry = FMath::Clamp(CurrentHungry - 1.f, 0.f, MaxHungry);

	if (HasAuthority())
		UpdateHungryWidget();

	if (CurrentHungry == 0 && CurrentHealth > 0)
		RemoveHealth(1.f);
}

void AGameplayCharacter::AddHungry(const int Amount)
{ // must be called only by server
	CurrentHungry = FMath::Clamp(CurrentHungry + Amount,
			0.f, MaxHungry);
	
	if (HasAuthority())
		UpdateHungryWidget();
}

void AGameplayCharacter::AddHealth(const int Amount)
{ // must be called only by server
	CurrentHealth = FMath::Clamp(CurrentHealth + Amount,
			0.f, MaxHealth);
	
	if (HasAuthority())
		UpdateHealthWidget();
}

void AGameplayCharacter::Server_OnSetPlayerController_Implementation()
{
	Client_OnSetPlayerController();

	GetWorldTimerManager().SetTimer(HungryTimerHandle,
		FTimerDelegate::CreateLambda([this] {
			RemoveHungry();
		}),
		5.f,
		true);
}

void AGameplayCharacter::OnHit()
{
	if (GetVelocity().Length() > 0)
		return;
	
	FVector EndLoc = GetActorLocation() + GetFollowCamera()->GetForwardVector();
	EndLoc.Z = GetActorLocation().Z;
	FRotator RotationToSet = UKismetMathLibrary::FindLookAtRotation(GetActorLocation(), EndLoc);
	RotationToSet.Yaw -= 90.f;
	Client_OnHit(RotationToSet);
	Server_OnHit(RotationToSet);
}

void AGameplayCharacter::Client_OnHit_Implementation(const FRotator& RotationToSet)
{
	GetMesh()->SetWorldRotation(RotationToSet);
	
	if (IsEquippedWeaponFireWeapon() &&
		!GetWorldTimerManager().IsTimerActive(LockAimTimerHandle))
	{
		GetWorldTimerManager().SetTimer(
			LockAimTimerHandle,
			FTimerDelegate::CreateLambda(
				[this]
				{
					if (!Hitting)
					{
						GetWorldTimerManager().ClearTimer(LockAimTimerHandle);
						return;
					}
					WeaponComponent->TryLockAim();
				}),
				0.01f,
				true);
	}
}

void AGameplayCharacter::Client_OnCraft_Implementation(const int ItemIndex)
{
	Server_TryCraft(ItemIndex);
}

void AGameplayCharacter::Server_TryCraft_Implementation(const int ItemIndex)
{
	InventoryComponent->TryCraft(ItemIndex);
}

void AGameplayCharacter::Server_OnHit_Implementation(const FRotator& RotationToSet)
{
	GetMesh()->SetWorldRotation(RotationToSet);
	
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
	return ItemInfo.OtherDataTableId;
}

void AGameplayCharacter::UpdateHealthWidget()
{
	if (HUDRef)
	{
		HUDRef->OnUpdateHealth(CurrentHealth);
	} else if (Cast<APlayerController>(GetController())) {
		if (Cast<APlayerController>(GetController())->GetHUD())
		{
			HUDRef = Cast<AGameplayHUD>(Cast<APlayerController>(GetController())->GetHUD());
			HUDRef->OnUpdateHealth(CurrentHealth);
		}
	}
}

void AGameplayCharacter::RemoveHealth(const int Amount)
{
	const float DamageMultiplierReduction = (100.f - static_cast<float>(CurrentDefensePoints))/100.f;
	CurrentHealth -= Amount * DamageMultiplierReduction;
	if (CurrentHealth < 0.f)
		CurrentHealth = 0.f;
	
	if (HasAuthority())
		UpdateHealthWidget();

	if (CurrentHealth <= 0.f)
		Die();
}

void AGameplayCharacter::Server_OnGetHitted_Implementation(const float Damage)
{
	RemoveHealth(Damage);
}

void AGameplayCharacter::OnRep_CurrentHealth()
{
	if (HUDRef)
		HUDRef->OnUpdateHealth(CurrentHealth);
}

void AGameplayCharacter::OnRep_CurrentHungry()
{
	if (HUDRef)
		HUDRef->OnUpdateHungry(CurrentHungry);
}

void AGameplayCharacter::Die()
{
	if (!InventoryComponent)
		return;

	GetWorldTimerManager().ClearTimer(HungryTimerHandle);
	
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

int AGameplayCharacter::GetEquippedWeaponId() const
{
	if(InventoryComponent->Slots[0].ItemInfo.ItemType == EItemType::WEAPON)
		return InventoryComponent->Slots[0].ItemInfo.OtherDataTableId;
	
	if (InventoryComponent->Slots[1].ItemInfo.ItemType == EItemType::WEAPON)
		return InventoryComponent->Slots[1].ItemInfo.OtherDataTableId;

	return -1;
}

bool AGameplayCharacter::CheckCanConvertItem(const int InventoryIndex) const
{
	return InventoryComponent->Slots[InventoryIndex].ItemInfo.ConvertTo != -1;
}

void AGameplayCharacter::SwapSlots(const int FirstSlotIndex, const int SecondSlotIndex)
{
	InventoryComponent->SwapSlots(FirstSlotIndex, SecondSlotIndex);
}

bool AGameplayCharacter::HasItem(const int ItemIndex) const
{
	return InventoryComponent->HasItem(ItemIndex);
}

int AGameplayCharacter::GetItemOnIndex(const int SlotIndex)
{
	return InventoryComponent->Slots[SlotIndex].ItemInfo.Index;
}

void AGameplayCharacter::SpawnPlaceableAhead(TSubclassOf<AActor> Class) const
{
	const FVector StartTraceLocation = GetActorLocation() + GetActorForwardVector() * 50.f + FVector(0.f, 0.f, 50.f);
	const FVector EndTraceLocation = StartTraceLocation - FVector(0.f, 0.f, 300.f);

	FHitResult OutHit;
	GetWorld()->LineTraceSingleByChannel(OutHit, StartTraceLocation, EndTraceLocation, ECC_Visibility);
	GetWorld()->SpawnActor<AActor>(Class, OutHit.ImpactPoint, FRotator(), FActorSpawnParameters());
}

bool AGameplayCharacter::IsEquippedWeaponFireWeapon() const
{	
	const int WeaponIdOnWeaponsDT = GetEquippedWeaponId();

	if (WeaponIdOnWeaponsDT == -1)
		return false;

	return InventoryComponent->IsFireWeapon(WeaponIdOnWeaponsDT);
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

void AGameplayCharacter::ConvertItem(const int InventoryIndex)
{
	Server_TryConvertItem(InventoryIndex);
}

void AGameplayCharacter::Server_TryConvertItem_Implementation(const int InventoryIndex)
{
	InventoryComponent->ConvertItem(InventoryIndex, 1, 1);
}

void AGameplayCharacter::UseItem(const int InventoryIndex)
{
	Server_TryUseItem(InventoryIndex);
}

void AGameplayCharacter::Server_TryUseItem_Implementation(const int InventoryIndex)
{
	if (InventoryComponent->ItemOnIndexIsOfType(InventoryIndex, EItemType::EQUIP))
	{
		EquipItemOnIndex(InventoryIndex);
		return;
	}

	if (InventoryComponent->ItemOnIndexIsOfType(InventoryIndex, EItemType::BACKPACK))
	{
		const FBackpackItem BackpackInfo = InventoryComponent->GetBackpackInfoFromSlotIndex(InventoryIndex);
		InventoryComponent->IncreaseNumberOfSlots(BackpackInfo.Slots);
		InventoryComponent->RemoveItem(InventoryIndex, 1);
		Backpack->SetStaticMesh(BackpackInfo.MeshRef);
		Backpack->SetRelativeTransform(BackpackInfo.TransformOnEquip);
		return;
	}

	if (InventoryComponent->ItemOnIndexIsOfType(InventoryIndex, EItemType::PLACEABLE))
	{
		const FPlaceableItem PlaceableInfo = InventoryComponent->GetPlaceableInfoFromSlotIndex(InventoryIndex);
		SpawnPlaceableAhead(PlaceableInfo.ClassToSpawn);
		InventoryComponent->RemoveItem(InventoryIndex, 1);
		return;
	}
	
	if (InventoryComponent->ItemOnIndexIsOfType(InventoryIndex, EItemType::WEAPON))
		return;

	if (InventoryComponent->UseItem(InventoryIndex))
		return;
}

void AGameplayCharacter::EquipItemOnIndex(const int InventoryIndex)
{
	const FEquipmentItem EquipmentInfo = InventoryComponent->GetEquipmentInfoFromSlotIndex(InventoryIndex);

	switch (EquipmentInfo.Type)
	{
	case EEquipmentType::HELMET:
		if (Helmet->GetSkeletalMeshAsset() != nullptr)
			return; // has already a helmet equipped
		Helmet->SetSkeletalMesh(EquipmentInfo.MeshRef);
		Multicast_ChangeEquipment(Helmet, EquipmentInfo.MeshRef);
		break;
	case EEquipmentType::CHEST:
		if (Chest->GetSkeletalMeshAsset() != nullptr)
			return; // has already a chest equipped
		Chest->SetSkeletalMesh(EquipmentInfo.MeshRef);
		Multicast_ChangeEquipment(Chest, EquipmentInfo.MeshRef);
		break;
	case EEquipmentType::PANTS:
		if (Pants->GetSkeletalMeshAsset() != nullptr)
			return; // has already a pants equipped
		Pants->SetSkeletalMesh(EquipmentInfo.MeshRef);
		Multicast_ChangeEquipment(Pants, EquipmentInfo.MeshRef);
		break;
	case EEquipmentType::SHOES:
		if (Shoes->GetSkeletalMeshAsset() != nullptr)
			return; // has already a shoes equipped
		Shoes->SetSkeletalMesh(EquipmentInfo.MeshRef);
		Multicast_ChangeEquipment(Shoes, EquipmentInfo.MeshRef);
		break;
	default:
		break;
	}

	CurrentDefensePoints += EquipmentInfo.DefensePoints;
	
	InventoryComponent->RemoveItem(InventoryIndex, 1);
}

void AGameplayCharacter::DropItem(const int SlotIndex)
{
	InventoryComponent->DropItem(SlotIndex);
}

void AGameplayCharacter::Multicast_ChangeEquipment_Implementation(USkeletalMeshComponent* SKMRef, USkeletalMesh* MeshToSet)
{
	SKMRef->SetSkeletalMesh(MeshToSet);
}

void AGameplayCharacter::AddItem(TPair<int, int> ItemToAdd) const
{
	InventoryComponent->GiveItem(ItemToAdd.Key, ItemToAdd.Value);
}

void AGameplayCharacter::OnDie()
{
	if(!SpawnerRef)
		return;
	SpawnerRef->OnLoseActor(this);	
}

void AGameplayCharacter::Destroyed()
{
	OnDie();
	
	Super::Destroyed();
}

void AGameplayCharacter::Client_OnCharacterGetCloseToChest_Implementation(AChest* ChestRef)
{
	ChestRef->SetWidgetVisibility(true);
}

void AGameplayCharacter::Client_OnCharacterGetFarToChest_Implementation(AChest* ChestRef)
{
	ChestRef->SetWidgetVisibility(false);
}

void AGameplayCharacter::Pickup()
{
	if (!ClosePickable)
		return;
	InventoryComponent->GiveItem(ClosePickable->ItemId, ClosePickable->Amount);
	ClosePickable->OnPick();
}

void AGameplayCharacter::OnUpdateInventory(TArray<FInventorySlot> Slots, const TArray<FWeaponInventorySlot>& WeaponSlots)
{
	if (Slots.Num() == 0)
		return;
	
	LeftHandItemComponent->SetStaticMesh(Slots[0].ItemInfo.Mesh);
	LeftHandItemComponent->SetRelativeTransform(Slots[0].ItemInfo.TransformOnHand);
	
	if (Slots.Num() < 2)
		return;
	
	RightHandItemComponent->SetStaticMesh(Slots[1].ItemInfo.Mesh);
	const FVector RightLoc = Slots[1].ItemInfo.TransformOnHand.GetLocation();
	const FRotator RightRot = Slots[1].ItemInfo.TransformOnHand.Rotator();
	const FVector RightScale = Slots[1].ItemInfo.TransformOnHand.GetScale3D();
	RightHandItemComponent->SetRelativeTransform(FTransform(RightRot, -RightLoc, RightScale));
}

void AGameplayCharacter::Server_SwapChestInventorySlots_Implementation(AChest* ChestRef, const int ChestIndex, const int InventoryIndex)
{
	const FInventorySlot InventorySlot = InventoryComponent->Slots[InventoryIndex];
	InventoryComponent->Slots[InventoryIndex] = ChestRef->Slots[ChestIndex];
	InventoryComponent->UpdateInventory();
	ChestRef->Slots[ChestIndex] = InventorySlot;
	ChestRef->UpdateInventory();
}

void AGameplayCharacter::Server_SwapChestSlots_Implementation(AChest* ChestRef, const int FirstChestIndex, const int SecondChestIndex)
{
	const FInventorySlot FirstSlot = ChestRef->Slots[FirstChestIndex];
	ChestRef->Slots[FirstChestIndex] = ChestRef->Slots[SecondChestIndex];
	ChestRef->Slots[SecondChestIndex] = FirstSlot;
	ChestRef->UpdateInventory();
}

void AGameplayCharacter::Server_DropChestSlot_Implementation(AChest* ChestRef, const int ChestIndex)
{
	const auto& [ItemInfo, Amount] = ChestRef->Slots[ChestIndex];

	FActorSpawnParameters SpawnInfo;
	const FVector LocationToSpawn = LeftHandItemComponent->GetComponentLocation();
	const FTransform TransformToSpawn(FTransform(FRotator(0), LocationToSpawn, FVector(1)));
	APickable* CurrentPickable = GetWorld()->SpawnActorDeferred<APickable>(InventoryComponent->PickableClass,
		TransformToSpawn,
		this,
		this,
		ESpawnActorCollisionHandlingMethod::AlwaysSpawn);
	CurrentPickable->ItemId = ItemInfo.Index;
	CurrentPickable->Amount = Amount;
	UGameplayStatics::FinishSpawningActor(CurrentPickable, TransformToSpawn);

	ChestRef->Slots[ChestIndex] = FInventorySlot();
	ChestRef->UpdateInventory();
}
