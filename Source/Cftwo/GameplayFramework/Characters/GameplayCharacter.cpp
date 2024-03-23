// Fill out your copyright notice in the Description page of Project Settings.

#include "GameplayCharacter.h"

#include "AIController.h"
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
#include "Cftwo/Actors/Chest.h"
#include "Cftwo/Actors/LootDrop.h"
#include "Cftwo/GameplayFramework/GameplayGameState.h"
#include "Cftwo/GameplayFramework/AI/BaseNeutralCharacter.h"
#include "Cftwo/Utils/ActorToSpawn.h"
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
	PrimaryActorTick.bCanEverTick = false;
	PrimaryActorTick.bStartWithTickEnabled = false;

	Helmet = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Helmet"));
	Helmet->SetupAttachment(GetMesh());
	Helmet->SetLeaderPoseComponent(GetMesh());
	Helmet->SetRenderCustomDepth(true);
	Helmet->PrimaryComponentTick.bStartWithTickEnabled = false; // does this make anim freeze?

	Chest = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Chest"));
	Chest->SetupAttachment(GetMesh());
	Chest->SetLeaderPoseComponent(GetMesh());
	Chest->SetRenderCustomDepth(true);
	Chest->PrimaryComponentTick.bStartWithTickEnabled = false; // does this make anim freeze?

	Pants = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Pants"));
	Pants->SetupAttachment(GetMesh());
	Pants->SetLeaderPoseComponent(GetMesh());
	Pants->SetRenderCustomDepth(true);
	Pants->PrimaryComponentTick.bStartWithTickEnabled = false; // does this make anim freeze?

	Shoes = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Shoes"));
	Shoes->SetupAttachment(GetMesh());
	Shoes->SetLeaderPoseComponent(GetMesh());
	Shoes->SetRenderCustomDepth(true);
	Shoes->PrimaryComponentTick.bStartWithTickEnabled = false; // does this make anim freeze?

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
	DOREPLIFETIME(AGameplayCharacter, Dead);
	DOREPLIFETIME(AGameplayCharacter, CurrentBuildings);
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

void AGameplayCharacter::BeginPlay()
{
	Super::BeginPlay();
	
	GameState = Cast<AGameplayGameState>(UGameplayStatics::GetGameState(GetWorld()));
}

void AGameplayCharacter::Server_StartSpawningActors_Implementation()
{
	if (GetController() == UGameplayStatics::GetPlayerController(GetWorld(), 0))
	{
		SpawnAllActors();
	
		FTimerHandle UnusedHandle;
		GetWorldTimerManager().SetTimer(UnusedHandle,
			this,
			&AGameplayCharacter::CheckShouldSpawnActors,
			TimeBetweenSpawnActors,
			true);
	
		FTimerHandle SecondUnusedHandle;
		GetWorldTimerManager().SetTimer(SecondUnusedHandle,
			this,
			&AGameplayCharacter::CheckShouldDespawnActors,
			TimeBetweenSpawnActors,
			true);
	}
}

void AGameplayCharacter::SpawnAllActors()
{
	for (int i = 0; i < ActorsToSpawn.Num(); i++)
	{
		ActorsSpawned.Add(FActorMatrix());

		for (int j=0; j < ActorsToSpawn[i].MinimumSpawned; j++)
		{
			SpawnActor(i);
		}
	}
}

bool AGameplayCharacter::SpawnActor(const int Index)
{
	const int RandomAngleIndex = FMath::RandRange(0, 359);
	const FVector SpawnDirection = GetActorForwardVector();
	const float CurrentSpawnAngle = RandomAngleIndex*SpawnAngle;
	const float XLocationNotRotated = SpawnDistance * SpawnDirection.X;
	const float YLocationNotRotated = SpawnDistance * SpawnDirection.Y;
	const float SpawnLocationX = GetActorLocation().X + XLocationNotRotated * cos(CurrentSpawnAngle) - YLocationNotRotated * sin(CurrentSpawnAngle);
	const float SpawnLocationY = GetActorLocation().Y + XLocationNotRotated * sin(CurrentSpawnAngle) + YLocationNotRotated * cos(CurrentSpawnAngle);
	const FVector SpawnLocation = FVector(SpawnLocationX, SpawnLocationY, GetActorLocation().Z);
		
	FVector StartLocation = SpawnLocation;
	StartLocation.Z = SpawnLocation.Z + 20000.f;
	FVector EndLocation = StartLocation + FVector(0.f, 0.f, -1000000.f); // this down because the hit can fail on spawn character

	FHitResult OutHit;
	if (GetWorld()->LineTraceSingleByChannel(OutHit,
		StartLocation, EndLocation,
		ECC_Visibility))
	{
		if (Cast<ABreakableObject>(OutHit.GetActor()))
			return false;
		
		TSubclassOf<AActor> ClassToSpawn = ActorsToSpawn[Index].ActorClass;
		FVector SpawnLoc = OutHit.ImpactPoint;
		SpawnLoc += FVector(0.f, 0.f, 100.f);
		FTransform SpawnTransform(FRotator(), SpawnLoc, FVector(1.f, 1.f, 1.f));

		FActorSpawnParameters SpawnParams;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
		AActor* ActorRef = GetWorld()->SpawnActorDeferred<AActor>(ClassToSpawn, SpawnTransform);
		if (Cast<AGameplayCharacter>(ActorRef))
		{
			Cast<AGameplayCharacter>(ActorRef)->CharacterSpawnerRef = this;
			GPrintDebug("Spawning NPC.");
		}
		else if (Cast<ABaseNeutralCharacter>(ActorRef))
		{
			GPrintDebug("Spawning Animal.");
			Cast<ABaseNeutralCharacter>(ActorRef)->CharacterSpawnerRef = this;
		}
		ActorsSpawned[Index].ActorArray.Add(ActorRef);
		UGameplayStatics::FinishSpawningActor(ActorRef, SpawnTransform);
		return true;
	}
	return false;
}

void AGameplayCharacter::CheckShouldSpawnActors()
{
	for (int i=0;i < ActorsToSpawn.Num();i++)
	{
		// Check if we have actors enough
		if (ActorsSpawned.Num() > i)
		{
			if (ActorsSpawned[i].ActorArray.Num() >= ActorsToSpawn[i].MinimumSpawned ||
				!ActorsToSpawn[i].CanRespawn)
					continue;
		}

		while(!SpawnActor(i));
	}
}

void AGameplayCharacter::SetRotationAccordingToVelocity(const float DeltaTime)
{
	if (GetVelocity().Length() < 10.f)
		return;
	
	FVector EndLoc = GetActorLocation() + GetVelocity();
	EndLoc.Z = GetActorLocation().Z;
	FRotator DesiredRotation = UKismetMathLibrary::FindLookAtRotation(GetActorLocation(), EndLoc);
	DesiredRotation.Yaw -= 90.f;
	const FRotator InterpedRotation = UKismetMathLibrary::RInterpTo(GetMesh()->GetComponentRotation(), DesiredRotation, DeltaTime, 8.f);
	GetMesh()->SetWorldRotation(InterpedRotation);
}

void AGameplayCharacter::Move(const FInputActionValue& Value)
{
	// Do not move while hitting
	if (Hitting)
		return;
	
	SetRotationAccordingToVelocity(GetWorld()->GetDeltaSeconds());
	
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
	if (AChest* ChestRef = Cast<AChest>(OtherActor))
	{
		Client_OnCharacterGetCloseToChest(ChestRef);
	}
}

void AGameplayCharacter::OnOverlapInteractable(AActor* OtherActor)
{
	CloseInteractable.Add(OtherActor);

	if (HUDRef && CloseInteractable.Num() == 1)
		HUDRef->OnPickableClose();
}

void AGameplayCharacter::OnEndOverlapInteractable(AActor* OtherActor)
{
	CloseInteractable.Remove(OtherActor);
	
	if (HUDRef && CloseInteractable.Num() == 0)
		HUDRef->OnPickableFar();

}

void AGameplayCharacter::OnComponentEndOverlap(class UPrimitiveComponent* OverlappedComp, class AActor* OtherActor, class UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (AChest* ChestRef = Cast<AChest>(OtherActor))
	{
		Client_OnCharacterGetFarToChest(ChestRef);
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

void AGameplayCharacter::HungryAndHealthTick()
{ // must be called only by server
	CurrentHungry = FMath::Clamp(CurrentHungry - 1.f, 0.f, MaxHungry);

	if (HasAuthority())
		UpdateHungryWidget();

	if (CurrentHungry == 0 && CurrentHealth > 0)
	{
		RemoveHealth(1.f);
	}
	else if (CurrentHungry > 0 && CurrentHealth > 0)
	{
		AddHealth(CurrentHungry*5.f/100.f);
	}
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
			HungryAndHealthTick();
		}),
		5.f,
		true);

	Server_StartSpawningActors();
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
	// Needed in order to timer work
	if (GetWorldTimerManager().IsTimerActive(HitTimerHandle))
		GetWorldTimerManager().ClearTimer(HitTimerHandle);
	GetWorldTimerManager().SetTimer(HitTimerHandle, this,
		&AGameplayCharacter::OnStopHitting,
		TIME_TO_STOP_HITTING,
		false);
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
	if (Dead || Invulnerable)
		return;
	
	const float DamageMultiplierReduction = (100.f - static_cast<float>(CurrentDefensePoints))/100.f;
	CurrentHealth -= Amount * DamageMultiplierReduction;
	if (CurrentHealth < 0.f)
		CurrentHealth = 0.f;
	
	if (HasAuthority())
		UpdateHealthWidget();

	if (CurrentHealth <= 0.f)
		Die();
}

void AGameplayCharacter::Client_OnGetHitted_Implementation()
{
	if (!HUDRef)
		return;
	
	Client_ShakeCamera();
	Client_PlaySound(SoundToFireWhenHitted);
	HUDRef->OnGetHitted();
}


void AGameplayCharacter::Server_OnGetHitted_Implementation(const float Damage)
{
	RemoveHealth(Damage);

	Client_OnGetHitted();
}

void AGameplayCharacter::Client_ShakeCamera_Implementation()
{
	APlayerController* MyController = Cast<APlayerController>(GetController());
	if (!MyController)
		return;
	if (!MyController->PlayerCameraManager)
		return;
	MyController->PlayerCameraManager->StartCameraShake(DefaultShakeClass,
		1.f);
}

void AGameplayCharacter::Client_PlaySound_Implementation(USoundBase* SoundToPlay)
{
	UGameplayStatics::PlaySound2D(GetWorld(), SoundToPlay);
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
	if (Cast<AAIController>(GetController()))
	{
		if (!InventoryComponent)
			return;
		InventoryComponent->DropAllItems();
	}

	GetWorldTimerManager().ClearTimer(HungryTimerHandle);
	
	Client_OnDie();
	Dead = true;

	FTimerHandle UnusedTimerHandle;
	GetWorldTimerManager().SetTimer(
		UnusedTimerHandle,
		FTimerDelegate::CreateLambda(
			[this]
			{
				if (GetController() != UGameplayStatics::GetPlayerController(GetWorld(), 0))
				{
					Destroy();
				} else
				{
					UGameplayStatics::SetGlobalTimeDilation(GetWorld(), 0.f);
				}
			}),
			1.5f,
			false);
}

void AGameplayCharacter::Client_OnDie_Implementation()
{
	APlayerController* ControllerRef = Cast<APlayerController>(GetController());
	if (ControllerRef) {
		AGameplayHUD* CharacterHUD = Cast<AGameplayHUD>(ControllerRef->GetHUD());
		CharacterHUD->OnDie();
	}
	BP_OnDie();
}

int AGameplayCharacter::GetEquippedWeaponId() const
{
	if(InventoryComponent->Slots[0].ItemInfo.ItemType == EItemType::WEAPON)
		return InventoryComponent->Slots[0].ItemInfo.OtherDataTableId;
	
	if (InventoryComponent->Slots[1].ItemInfo.ItemType == EItemType::WEAPON)
		return InventoryComponent->Slots[1].ItemInfo.OtherDataTableId;

	return -1;
}

bool AGameplayCharacter::WeaponOnRightHand() const
{
	if(InventoryComponent->Slots[0].ItemInfo.ItemType == EItemType::WEAPON)
		return false;
	
	if (InventoryComponent->Slots[1].ItemInfo.ItemType == EItemType::WEAPON)
		return true;

	return false;
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
	
	UGameplayStatics::PlaySound2D(GetWorld(),
		SoundToFireWhenPlaceItem);
}

void AGameplayCharacter::Client_OnBuyBuilding_Implementation(const int BuildingIndex)
{
	HUDRef->OnBuyBuilding(BuildingIndex);
}

void AGameplayCharacter::Client_OnUseBuilding_Implementation(const int BuildingIndex)
{
	HUDRef->OnUseBuilding(BuildingIndex);
}

void AGameplayCharacter::Server_OnTryBuyBuilding_Implementation(const int BuildingIndex)
{
	if (GameState->Coins <= 0)
		return;

	GameState->Coins--;
	for (int i = 0; i < CurrentBuildings.Num(); i++)
	{
		if(CurrentBuildings[i].First != BuildingIndex)
			continue;

		// Adding amount if found and returning
		CurrentBuildings[i].Second++;
		Client_OnBuyBuilding(BuildingIndex);
		return;
	}

	// If not found, add to array
	CurrentBuildings.Add({BuildingIndex, 1});
	Client_OnBuyBuilding(BuildingIndex);
}

void AGameplayCharacter::OnTryBuyBuilding(const int BuildingIndex)
{
	Server_OnTryBuyBuilding(BuildingIndex);
}

void AGameplayCharacter::Server_RemoveBuilding_Implementation(const int BuildingIndex)
{
	int CurrentIndex = -1;
	bool Remove = false;
	for (auto [index, amount] : CurrentBuildings)
	{
		CurrentIndex++;
		if (index != BuildingIndex)
			continue;

		CurrentBuildings[CurrentIndex].Second--;
		if (CurrentBuildings[CurrentIndex].Second == 0)
			Remove = true;
		break;
	}
	if (Remove)
		CurrentBuildings.RemoveAt(CurrentIndex);
	Client_OnUseBuilding(BuildingIndex);
}

void AGameplayCharacter::RemoveBuilding(const int BuildingIndex)
{
	Server_RemoveBuilding(BuildingIndex);
}

void AGameplayCharacter::OnContinue()
{
	Dead = false;
	AddHealth(100.f);
	Invulnerable = true;
	IsAdAvailableToContinue = false;
	FTimerHandle UnusedHandle;
	GetWorldTimerManager().SetTimer(UnusedHandle,
		FTimerDelegate::CreateLambda([this]
		{
			Invulnerable = false;
		}),
		5.f,
		false);
}

void AGameplayCharacter::CheckShouldDespawnActors()
{
	for (FActorMatrix CurrentActorMatrix : ActorsSpawned)
	{
		int i = 0;
		while (i < CurrentActorMatrix.ActorArray.Num())
		{
			AActor* CurrentActor = CurrentActorMatrix.ActorArray[i];
			FVector2D CurrentActorLoc = FVector2D(CurrentActor->GetActorLocation().X, CurrentActor->GetActorLocation().Y);
			FVector2D SelfLoc = FVector2D(GetActorLocation().X, GetActorLocation().Y);
			if (FVector2D::Distance(CurrentActorLoc, SelfLoc) > DespawnDistance)
			{
				CurrentActor->Destroy();
				return; // Destroying one per loop because array need to be updated
			}
			i++;
		}
	}
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
		} else if (ALootDrop* CurrentLootDrop = Cast<ALootDrop>(CurrentHit.GetActor()))
		{
			CurrentLootDrop->OnInteract(this);
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
		Beckpack->SetSkeletalMesh(BackpackInfo.MeshRef);
		Beckpack->SetRelativeTransform(BackpackInfo.TransformOnEquip);
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

void AGameplayCharacter::AddOrDropItem(TPair<int, int> ItemToAdd)
{
	const int InventoryIndex = InventoryComponent->GiveItem(ItemToAdd.Key, ItemToAdd.Value);
	if (InventoryIndex == -1)
	{
		const FInventoryItem ItemInfo = InventoryComponent->GetItemInfo(ItemToAdd.Key);
		SpawnItemOnHand(ItemInfo, ItemToAdd.Value);
	}
}

void AGameplayCharacter::OnLoseActor(AActor* ActorRef)
{
	for (int i = 0; i < ActorsSpawned.Num(); i++)
	{
		if (ActorsSpawned[i].ActorArray.Contains(ActorRef))
		{
			ActorsSpawned[i].ActorArray.Remove(ActorRef);
			return;
		}
	}
}

void AGameplayCharacter::OnDie()
{
	if(!SpawnerRef)
	{
		if (!CharacterSpawnerRef)
			return;
		CharacterSpawnerRef->OnLoseActor(this);
		return;
	}
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
	if (CloseInteractable.Num() == 0)
		return;

	if (APickable* FoundPickable = Cast<APickable>(CloseInteractable[0]))
	{
		const int InventoryIndex = InventoryComponent->GiveItem(Cast<APickable>(CloseInteractable[0])->ItemId, Cast<APickable>(CloseInteractable[0])->Amount);
		if (InventoryIndex == -1)
			return;
	
		FoundPickable->OnPick();
		if (CloseInteractable.Contains(FoundPickable))
			CloseInteractable.Remove(FoundPickable);
	} else if (ALootDrop* FoundLootDrop = Cast<ALootDrop>(CloseInteractable[0]))
	{
		FoundLootDrop->OnInteract(this);
		if (CloseInteractable.Contains(FoundLootDrop))
			CloseInteractable.Remove(FoundLootDrop);
	}
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
	// const FVector RightLoc = Slots[1].ItemInfo.TransformOnHand.GetLocation();
	// const FRotator RightRot = Slots[1].ItemInfo.TransformOnHand.Rotator();
	// const FVector RightScale = Slots[1].ItemInfo.TransformOnHand.GetScale3D();
	RightHandItemComponent->SetRelativeTransform(Slots[1].ItemInfo.TransformOnHand);
}

void AGameplayCharacter::Server_SwapChestInventorySlots_Implementation(AChest* ChestRef, const int ChestIndex, const int InventoryIndex, const bool FromChest)
{
	// Checking if are the same ids
	if (InventoryComponent->Slots[InventoryIndex].ItemInfo.Index ==
		ChestRef->Slots[ChestIndex].ItemInfo.Index)
	{
		FInventorySlot* FromSlot;
		FInventorySlot* ToSlot;
		if (FromChest)
		{
			ToSlot = &(InventoryComponent->Slots[InventoryIndex]);
			FromSlot = &(ChestRef->Slots[ChestIndex]);
		} else
		{		
			ToSlot = &(ChestRef->Slots[ChestIndex]);
			FromSlot = &(InventoryComponent->Slots[InventoryIndex]);	
		}
		
		const int MaxStack = ToSlot->ItemInfo.MaxStack;
		ToSlot->Amount += FromSlot->Amount;
		if (ToSlot->Amount > MaxStack)
		{
			const int Remaining = ToSlot->Amount - MaxStack;
			ToSlot->Amount = MaxStack;
			FromSlot->Amount = Remaining;
		} else
		{
			FromSlot->ItemInfo = FInventoryItem();
			FromSlot->Amount = 0;
		}
	} else
	{
		const FInventorySlot InventorySlot = InventoryComponent->Slots[InventoryIndex];
		InventoryComponent->Slots[InventoryIndex] = ChestRef->Slots[ChestIndex];
		ChestRef->Slots[ChestIndex] = InventorySlot;
	}
	ChestRef->UpdateInventory();
	InventoryComponent->UpdateInventory();
}

void AGameplayCharacter::Server_SwapChestSlots_Implementation(AChest* ChestRef, const int FirstChestIndex, const int SecondChestIndex)
{
	// Checking if are the same ids
	if (ChestRef->Slots[FirstChestIndex].ItemInfo.Index == ChestRef->Slots[SecondChestIndex].ItemInfo.Index)
	{
		FInventorySlot* ToSlot = &ChestRef->Slots[FirstChestIndex];
		FInventorySlot* FromSlot = &ChestRef->Slots[SecondChestIndex];

		const int MaxStack = ToSlot->ItemInfo.MaxStack;
		ToSlot->Amount += FromSlot->Amount;
		if (ToSlot->Amount > MaxStack)
		{
			const int Remaining = ToSlot->Amount - MaxStack;
			ToSlot->Amount = MaxStack;
			FromSlot->Amount = Remaining;
		} else
		{
			FromSlot->ItemInfo = FInventoryItem();
			FromSlot->Amount = 0;
		}
	} else
	{
		const FInventorySlot FirstSlot = ChestRef->Slots[FirstChestIndex];
		ChestRef->Slots[FirstChestIndex] = ChestRef->Slots[SecondChestIndex];
		ChestRef->Slots[SecondChestIndex] = FirstSlot;
	}
	ChestRef->UpdateInventory();
}

void AGameplayCharacter::SpawnItemOnHand(FInventoryItem ItemInfo, const int Amount)
{
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
}

void AGameplayCharacter::Server_DropChestSlot_Implementation(AChest* ChestRef, const int ChestIndex)
{
	const auto& [ItemInfo, Amount] = ChestRef->Slots[ChestIndex];

	SpawnItemOnHand(ItemInfo, Amount);

	ChestRef->Slots[ChestIndex] = FInventorySlot();
	ChestRef->UpdateInventory();
}

void AGameplayCharacter::Server_OnHitSuccess_Implementation()
{
	Client_OnHitSuccess();
}

void AGameplayCharacter::Client_OnHitSuccess_Implementation()
{
	UGameplayStatics::PlaySound2D(GetWorld(),
		SoundToFireWhenHitSuccess);

	if (HUDRef)
		HUDRef->OnHitSuccess();
}

void AGameplayCharacter::Client_OnHitWithoutRightWeapon_Implementation()
{
	if (HUDRef)
		HUDRef->OnMistakenWeapon();
}
