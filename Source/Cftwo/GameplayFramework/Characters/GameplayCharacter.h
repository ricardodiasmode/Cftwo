// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "../../CftwoCharacter.h"
#include "GameplayCharacter.generated.h"

class UCameraComponent;
class USpringArmComponent;

UCLASS()
class CFTWO_API AGameplayCharacter : public ACftwoCharacter
{
	GENERATED_BODY()

private:
	// Controlls the time that player will stop hit after the hit starts.
	// Note that this time may follow anim hit time
	static constexpr auto TIME_TO_STOP_HITTING = 0.44f;

	// Handle OnStopHitting function
	FTimerHandle HitTimerHandle;

	/** Hit Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
		class UInputAction* HitAction;

public:
	// Controlls whether or not player is hitting
	UPROPERTY(BlueprintReadOnly, Replicated)
		bool m_Hitting = false;

private:
	// Trigger player hitting status and set timer for stop hitting
	UFUNCTION(Server, Reliable)
	void Server_OnHit();
	// Handle for server
	void OnHit();
	// Set flag telling that character needs to stop hitting
	void OnStopHitting();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	/** Called for movement input */
	void Move(const FInputActionValue& Value) override;

public:
	// Sets default values for this character's properties
	AGameplayCharacter();

	//// Called every frame
	//virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

};
