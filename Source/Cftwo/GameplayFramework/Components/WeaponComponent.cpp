// Fill out your copyright notice in the Description page of Project Settings.


#include "WeaponComponent.h"
#include "../Characters/GameplayCharacter.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Kismet/KismetMathLibrary.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "../AI/BaseNeutralCharacter.h"
#include "../../Actors/BreakableObject.h"
#include "Inventory/InventoryComponent.h"
#include "Camera/CameraComponent.h"
#include "Kismet/GameplayStatics.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraComponent.h"

// Sets default values for this component's properties
UWeaponComponent::UWeaponComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}


// Called when the game starts
void UWeaponComponent::BeginPlay()
{
	Super::BeginPlay();

	// ...

}

bool UWeaponComponent::CanHit()
{
	if (!m_CanHit)
		return false;

	static constexpr auto UNUSED_LOOP_TIME = 0.1f;

	m_CanHit = false;
	FTimerHandle UnusedTimer;
	CharacterRef->GetWorldTimerManager().SetTimer(UnusedTimer, this,
		&UWeaponComponent::SetCanHit, UNUSED_LOOP_TIME, false,
		CurrentHitInterval - UNUSED_LOOP_TIME);
	return true;
}

void UWeaponComponent::SetCanHit()
{
	m_CanHit = true;
}

void UWeaponComponent::OnHit()
{
	if (!CanHit())
		return;

	if (GetEquippedWeaponByIndex(m_CurrentWeapon) == -1)
		OnPunch();
	else
		TryFireWeapon();
}

UNiagaraComponent* UWeaponComponent::SpawnVFXAtLocation(UNiagaraSystem* VFXToSpawn, const FVector& LocationToSpawn, const FRotator& RotationToSpawn)
{
	return UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(),
	                                               VFXToSpawn,
	                                               LocationToSpawn,
	                                               RotationToSpawn,
	                                               FVector(1.f),
	                                               true,
	                                               true);
}

void UWeaponComponent::SpawnVFXOnAttack(FHitResult CurrentHit, AActor* Target, UNiagaraSystem* VFX)
{
	FVector DesiredLoc = Target->GetActorLocation();
	FVector VFXInitialTraceLoc = CurrentHit.Location;
					
	TArray<FHitResult> VFXOutHit;
	GetWorld()->LineTraceMultiByChannel(VFXOutHit,
	                                    VFXInitialTraceLoc,
	                                    DesiredLoc,
	                                    ECC_Visibility);

	bool VFXSpawned = false;
	for (FHitResult CurrentVFXOutHit : VFXOutHit)
	{
		if (CurrentVFXOutHit.GetActor() == Target)
		{
			UNiagaraComponent* SpawnedNiagara = SpawnVFXAtLocation(VFX, CurrentVFXOutHit.ImpactPoint, CurrentVFXOutHit.ImpactNormal.Rotation());
			SpawnedNiagara->AttachToComponent(Target->GetRootComponent(), FAttachmentTransformRules::KeepWorldTransform);
			VFXSpawned = true;
			break;
		}
	}
	if (!VFXSpawned)
	{
		SpawnVFXAtLocation(VFX, CurrentHit.Location, CurrentHit.ImpactNormal.Rotation());
	}
}

void UWeaponComponent::OnPunch()
{
	FVector START_LOCATION = CharacterRef->GetActorLocation() +
		CharacterRef->GetActorForwardVector() * 75.f;
	FVector END_LOCATION = START_LOCATION;
	float RADIUS = 60.f;
	float HALF_HEIGHT = 90.f;
	TArray<TEnumAsByte<EObjectTypeQuery>> ObjTypes;
	ObjTypes.Add(UEngineTypes::ConvertToObjectType(ECC_Pawn));
	ObjTypes.Add(UEngineTypes::ConvertToObjectType(ECC_WorldStatic));
	TArray<AActor*> ActorsToIgnore;
	ActorsToIgnore.Add(CharacterRef);
	TArray<FHitResult> OutHits;
	if (UKismetSystemLibrary::CapsuleTraceMultiForObjects(
		GetWorld(),
		START_LOCATION,
		END_LOCATION,
		RADIUS,
		HALF_HEIGHT,
		ObjTypes,
		false,
		ActorsToIgnore,
		EDrawDebugTrace::None,
		OutHits,
		true
	)) {
		for (FHitResult CurrentHit : OutHits) {

			// If target is character
			if (AGameplayCharacter* CurrentCharacter = Cast<AGameplayCharacter>(CurrentHit.GetActor()))
			{
				CurrentCharacter->Server_OnGetHitted(PUNCH_DAMAGE);
				SpawnVFXOnAttack(CurrentHit, CurrentCharacter, CharacterRef->BloodVFX);
				return;
			}

			// If target is IA
			if (ABaseNeutralCharacter* CurrentIA = Cast<ABaseNeutralCharacter>(CurrentHit.GetActor()))
			{
				SpawnVFXOnAttack(CurrentHit, CurrentIA, CharacterRef->BloodVFX);
				
				if (CurrentIA->AmIAlive())
					CurrentIA->Server_OnGetHitted(PUNCH_DAMAGE, GetOwner());
				else
				{
					TArray<TPair<int, int>> ItemsToAdd = CurrentIA->OnHarvest();
					for (TPair<int,int> ItemToAdd : ItemsToAdd)
						CharacterRef->AddItem(ItemToAdd);
				}
				return;
			}

			// If target is breakable
			if (ABreakableObject* BreakableObject = Cast<ABreakableObject>(CurrentHit.GetActor()))
			{
				BreakableObject->RemoveHP();
				CharacterRef->InventoryComponent->GiveItem(BreakableObject->ItemToGive, 1);

				SpawnVFXOnAttack(CurrentHit, BreakableObject, CharacterRef->DustVFX);

				return;
			}
		}
	}
}

void UWeaponComponent::ChangeEquippedWeapon(const bool Forward)
{
	if (Forward)
		m_CurrentWeapon = FMath::Clamp(m_CurrentWeapon + 1, -1, 5);
	else
		m_CurrentWeapon = FMath::Clamp(m_CurrentWeapon - 1, -1, 5);

	OnChangeEquippedWeapon();
}

void UWeaponComponent::SetCurrentWeapon(const int SlotIndex)
{
	m_CurrentWeapon = SlotIndex;
	OnChangeEquippedWeapon();
}

void UWeaponComponent::OnChangeEquippedWeapon()
{
	FireWeaponEquipped = CharacterRef->IsEquippedWeaponFireWeapon();
	if (FireWeaponEquipped) {
		int EquippedWeaponId = CharacterRef->GetEquippedWeaponId();
		CharacterRef->OnWeaponChange(
			CharacterRef->GetWeaponInfo(EquippedWeaponId).Mesh,
			CharacterRef->GetWeaponInfo(EquippedWeaponId).AttachTransform
			);
	}
	else {
		CharacterRef->OnWeaponChange(nullptr, FTransform());
	}
}

void UWeaponComponent::TryFireWeapon()
{
	int EquippedWeaponId = CharacterRef->GetEquippedWeaponId();

	FWeaponItem WeaponInfo = CharacterRef->GetWeaponInfo(EquippedWeaponId);
	if (CharacterRef->IsEquippedWeaponFireWeapon())
	{
		Client_SpawnProjectile(WeaponInfo.ProjectileClassToSpawn);
	}
	else {
		// AttackWithMeleeWeapon();
	}
}

void UWeaponComponent::Client_SpawnProjectile_Implementation(TSubclassOf<ABaseProjectile> ProjectileToSpawnClass)
{
	Server_SpawnProjectile(CharacterRef->GetFollowCamera()->GetForwardVector(),
		CharacterRef->GetFollowCamera()->GetComponentLocation(),
		ProjectileToSpawnClass);
}

void UWeaponComponent::Server_SpawnProjectile_Implementation(FVector CameraForward, FVector CameraLoc, TSubclassOf<ABaseProjectile> ProjectileToSpawnClass)
{
	FActorSpawnParameters SpawnInfo;
	FVector SpawnLoc = CharacterRef->GetActorLocation() + CharacterRef->GetActorForwardVector() * 60.f;

	FHitResult OutHit;
	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(CharacterRef);
	GetWorld()->LineTraceSingleByChannel(
		OutHit,
		CameraLoc,
		CameraLoc + (CameraForward * 10000.f),
		ECC_Visibility,
		QueryParams
	);

	FRotator SpawnRot = CharacterRef->GetActorRotation();
	FVector ActorForward = CharacterRef->GetActorForwardVector();
	FVector FinalFirstHitLoc = OutHit.bBlockingHit ? OutHit.ImpactPoint : OutHit.TraceEnd;
	FinalFirstHitLoc += (FinalFirstHitLoc - SpawnLoc); // adding size 
	const bool HitOnFrontOfCharacter = (ActorForward.Dot(FinalFirstHitLoc - CharacterRef->GetActorLocation()) > 0);
	if (HitOnFrontOfCharacter)
	{
		GetWorld()->LineTraceSingleByChannel(
			OutHit,
			SpawnLoc,
			FinalFirstHitLoc,
			ECollisionChannel::ECC_Visibility,
			QueryParams
		);
		
		FVector FinalLoc = OutHit.bBlockingHit ? OutHit.ImpactPoint : OutHit.TraceEnd;
		SpawnRot = UKismetMathLibrary::FindLookAtRotation(SpawnLoc, FinalLoc);
	}

	FTransform TransformToSpawn(FTransform(SpawnRot, SpawnLoc, FVector(1)));
	ABaseProjectile* ProjectileRef = GetWorld()->SpawnActorDeferred<ABaseProjectile>(ProjectileToSpawnClass, TransformToSpawn, CharacterRef, CharacterRef);
	UGameplayStatics::FinishSpawningActor(ProjectileRef, TransformToSpawn);

	CharacterRef->SetActorRotation(FRotator(0.f, SpawnRot.Yaw, 0.f));
}

int UWeaponComponent::GetEquippedWeaponByIndex(const int Id)
{
	return CharacterRef->GetWeaponIdOnSlot(Id);
}

void UWeaponComponent::TryLockAim()
{
	FActorSpawnParameters SpawnInfo;
	FVector CameraLoc = CharacterRef->GetFollowCamera()->GetComponentLocation();
	FVector CameraForward = CharacterRef->GetFollowCamera()->GetForwardVector();
	TArray<FHitResult> OutHits;
	TArray<AActor*> ActorsToIgnore;
	ActorsToIgnore.Add(CharacterRef);
	TArray<TEnumAsByte<EObjectTypeQuery>> ObjTypes;
	ObjTypes.Add(UEngineTypes::ConvertToObjectType(ECC_Pawn));

	UKismetSystemLibrary::CapsuleTraceMultiForObjects(
		GetWorld(),
		CameraLoc,
		CameraLoc + (CameraForward * 10000.f),
		30.f,
		30.f,
		ObjTypes,
		false,
		ActorsToIgnore,
		EDrawDebugTrace::None,
		OutHits,
		true
		);

	for (FHitResult CurrentHit : OutHits)
	{
		if (!CurrentHit.bBlockingHit)
			continue;

		FVector EndLoc;
		if (Cast<AGameplayCharacter>(CurrentHit.GetActor()))
		{
			AGameplayCharacter* EnemyRef = Cast<AGameplayCharacter>(CurrentHit.GetActor());
			EndLoc = EnemyRef->GetLockPoint();
		} else if (Cast<ABaseNeutralCharacter>(CurrentHit.GetActor()))
		{
			ABaseNeutralCharacter* EnemyRef = Cast<ABaseNeutralCharacter>(CurrentHit.GetActor());
			EndLoc = EnemyRef->GetLockPoint();
		}

		FRotator RotationToSet = UKismetMathLibrary::FindLookAtRotation(CameraLoc, EndLoc);
		FRotator CurrentRotation = CharacterRef->GetController()->GetControlRotation();
		FRotator InterpedRot = UKismetMathLibrary::RInterpTo(CurrentRotation, RotationToSet, 0.1f, 2.f);
		CharacterRef->GetController()->SetControlRotation(InterpedRot);
		return;
	}
}
