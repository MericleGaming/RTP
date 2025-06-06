// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "BaseEnemy.generated.h"

// Forward declarations
class UAnimMontage;
class UPawnSensingComponent;
class USoundBase;
class UAudioComponent;

// Enemy states enum
UENUM(BlueprintType)
enum class EEnemyState : uint8
{
    Idle,
    Investigating,
    Chasing,
    Attacking,
    Stunned,
    Dead
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnEnemyDeath);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnHealthChanged, float, CurrentHealth, float, MaxHealth);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnStateChanged, EEnemyState, NewState);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnEnemySpotPlayer, AActor*, SpottedActor);

UCLASS()
class RTP_API ABaseEnemy : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	ABaseEnemy();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	// Health variables
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Health")
	float MaxHealth = 100.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Health")
	float CurrentHealth;

	// Death animation montage
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
	UAnimMontage* DeathMontage;
	
	// Attack animation montage
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
	UAnimMontage* AttackMontage;
	
	// Stun animation montage
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
	UAnimMontage* StunMontage;

	// Death timer handle for cleanup
	FTimerHandle DeathTimerHandle;
		// Stun timer handle
	FTimerHandle StunTimerHandle;
	
	// Attack cooldown timer
	FTimerHandle AttackCooldownTimerHandle;

	// Default death cleanup time
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Health")
	float DeathCleanupTime = 3.0f;
	
	// Default stun duration
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	float StunDuration = 3.0f;

	// Flag to check if enemy is dead
	UPROPERTY(BlueprintReadOnly, Category = "Health")
	bool bIsDead = false;
	
	// Current enemy state
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI")
	EEnemyState CurrentState = EEnemyState::Idle;
	
	// Sensing component for detecting players
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI")
	UPawnSensingComponent* PawnSensingComponent;
	
	// Detection range
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
	float SightRadius = 1000.0f;
	
	// Peripheral vision angle in degrees (total angle)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
	float SightAngle = 90.0f;
	
	// Hearing range
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
	float HearingRange = 800.0f;
	
	// How long enemy remembers player's last location
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
	float MemoryDuration = 7.0f;
	
	// Movement speeds
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float DefaultSpeed = 200.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float ChaseSpeed = 500.0f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float InvestigateSpeed = 300.0f;
	
	// Last known player location
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI")
	FVector LastKnownPlayerLocation;
	
	// Whether enemy is currently in attack cooldown
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat")
	bool bIsAttackOnCooldown = false;
	
	// Attack cooldown duration
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	float AttackCooldown = 2.0f;
	
	// Attack range
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	float AttackRange = 150.0f;
	
	// Attack damage
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	float AttackDamage = 20.0f;
	
	// Sound effects
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
	USoundBase* AttackSound;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
	USoundBase* DeathSound;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
	USoundBase* SpotPlayerSound;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
	USoundBase* StunnedSound;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
	USoundBase* IdleSound;
	
	// Audio component for playing sounds
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio")
	UAudioComponent* AudioComponent;
	
	// Whether this enemy is affected by flashlight
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
	bool bAffectedByFlashlight = true;
	
	// How much this enemy is affected by flashlight (higher = more affected)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
	float FlashlightSensitivity = 1.0f;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	// Handle damage received
	UFUNCTION(BlueprintCallable, Category = "Health")
	virtual float TakeDamageCustom(float DamageAmount, bool bIgnoreInvulnerability = false);

	// Override the engine's TakeDamage function
	virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, 
		class AController* EventInstigator, AActor* DamageCauser) override;

	// Function to heal the enemy
	UFUNCTION(BlueprintCallable, Category = "Health")
	virtual void HealEnemy(float HealAmount);

	// Handle enemy death
	UFUNCTION(BlueprintCallable, Category = "Health")
	virtual void Die();

	// Check if enemy is dead
	UFUNCTION(BlueprintPure, Category = "Health")
	bool IsDead() const { return bIsDead; }

	// Get current health percentage
	UFUNCTION(BlueprintPure, Category = "Health")
	float GetHealthPercent() const { return CurrentHealth / MaxHealth; }
	
	// Set the enemy state
	UFUNCTION(BlueprintCallable, Category = "AI")
	virtual void SetEnemyState(EEnemyState NewState);
	
	// Get the current enemy state
	UFUNCTION(BlueprintPure, Category = "AI")
	EEnemyState GetEnemyState() const { return CurrentState; }
	
	// Handle being stunned
	UFUNCTION(BlueprintCallable, Category = "Combat")
	virtual void Stun(float Duration = -1.0f);
	
	// End stun state
	UFUNCTION(BlueprintCallable, Category = "Combat")
	virtual void EndStun();
	
	// Perform attack
	UFUNCTION(BlueprintCallable, Category = "Combat")
	virtual void PerformAttack();
	
	// Start attack cooldown
	UFUNCTION(BlueprintCallable, Category = "Combat")
	virtual void StartAttackCooldown();
		// Check if in attack range of target
	UFUNCTION(BlueprintPure, Category = "Combat")
	bool IsInAttackRange(AActor* Target) const;
	
	// Move to location (basic navigation without pathfinding)
	UFUNCTION(BlueprintCallable, Category = "AI|Movement")
	virtual void MoveToLocation(const FVector& Location);
	
	// React to sound stimulus
	UFUNCTION(BlueprintCallable, Category = "AI")
	virtual void ReactToSound(AActor* SoundSource, const FVector& SoundLocation);
	
	// React to seeing player
	UFUNCTION(BlueprintCallable, Category = "AI")
	virtual void ReactToSeeingPlayer(APawn* PlayerPawn);
	
	// Check line of sight to target
	UFUNCTION(BlueprintCallable, Category = "AI")
	bool HasLineOfSightTo(AActor* Target) const;
	
	// Reset to default behavior
	UFUNCTION(BlueprintCallable, Category = "AI")
	virtual void ReturnToDefaultBehavior();
	
	// Get whether flashlight affects this enemy type
	UFUNCTION(BlueprintPure, Category = "AI")
	virtual bool IsAffectedByFlashlight() const { return bAffectedByFlashlight; }
	
	// Handle being hit by flashlight
	UFUNCTION(BlueprintCallable, Category = "AI")
	virtual void ReactToFlashlight(float Intensity);

	// Delegates for blueprint events
	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnEnemyDeath OnEnemyDeath;

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnHealthChanged OnHealthChanged;
	
	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnStateChanged OnStateChanged;
	
	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnEnemySpotPlayer OnEnemySpotPlayer;
	
protected:
	// Called when player is sensed
	UFUNCTION()
	virtual void OnPlayerSeen(APawn* Pawn);
	
	// Called when a sound is heard
	UFUNCTION()
	virtual void OnNoiseHeard(APawn* NoiseInstigator, const FVector& Location, float Volume);
};
