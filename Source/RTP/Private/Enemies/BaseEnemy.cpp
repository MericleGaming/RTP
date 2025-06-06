// Fill out your copyright notice in the Description page of Project Settings.


#include "Enemies/BaseEnemy.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Components/CapsuleComponent.h"
#include "Perception/PawnSensingComponent.h"
#include "NavigationSystem.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundBase.h"

// Sets default values
ABaseEnemy::ABaseEnemy()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
    
    // Initialize health to max health
    CurrentHealth = MaxHealth;
    
    // Set up the pawn sensing component
    PawnSensingComponent = CreateDefaultSubobject<UPawnSensingComponent>(TEXT("PawnSensingComponent"));
    PawnSensingComponent->SetPeripheralVisionAngle(SightAngle);
    PawnSensingComponent->SightRadius = SightRadius;
    PawnSensingComponent->HearingThreshold = HearingRange;
    PawnSensingComponent->LOSHearingThreshold = HearingRange * 0.75f;
    PawnSensingComponent->bOnlySensePlayers = true;
    
    // Set up audio component
    AudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("AudioComponent"));
    AudioComponent->SetupAttachment(RootComponent);
    AudioComponent->bAutoActivate = false;
}

// Called when the game starts or when spawned
void ABaseEnemy::BeginPlay()
{
	Super::BeginPlay();
	
    // Set health to max at the beginning of the game
    CurrentHealth = MaxHealth;
    
    // Broadcast initial health
    OnHealthChanged.Broadcast(CurrentHealth, MaxHealth);
    
    // Set up perception component delegates
    if (PawnSensingComponent)
    {
        PawnSensingComponent->OnSeePawn.AddDynamic(this, &ABaseEnemy::OnPlayerSeen);
        PawnSensingComponent->OnHearNoise.AddDynamic(this, &ABaseEnemy::OnNoiseHeard);
    }
    
    // Initialize with idle state
    SetEnemyState(EEnemyState::Idle);
}

// Called every frame
void ABaseEnemy::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
    
    // Handle state-specific behaviors
    if (!bIsDead)
    {
        AAIController* AIController = Cast<AAIController>(GetController());
        
        switch (CurrentState)
        {
            case EEnemyState::Chasing:
                // Check if we're in attack range of the player
                if (AActor* Player = UGameplayStatics::GetPlayerPawn(this, 0))
                {
                    if (IsInAttackRange(Player) && !bIsAttackOnCooldown)
                    {
                        PerformAttack();
                    }
                    else if (HasLineOfSightTo(Player))
                    {
                        // Update last known location if we can see the player
                        LastKnownPlayerLocation = Player->GetActorLocation();
                        
                        // Move to the player
                        if (AIController)
                        {
                            AIController->MoveToActor(Player);
                        }
                    }
                    else
                    {
                        // Move to last known location if we can't see the player
                        MoveToLocation(LastKnownPlayerLocation);
                        
                        // Switch to investigating if we've lost sight
                        GetWorldTimerManager().SetTimer(
                            StunTimerHandle, // Reuse stun timer for this
                            [this]()
                            {
                                SetEnemyState(EEnemyState::Investigating);
                            },
                            MemoryDuration,
                            false
                        );
                    }
                }
                break;
                
            case EEnemyState::Investigating:
                // Check if player is nearby during investigation
                if (AActor* Player = UGameplayStatics::GetPlayerPawn(this, 0))
                {
                    if (HasLineOfSightTo(Player))
                    {
                        // If we can see the player again, go back to chasing
                        SetEnemyState(EEnemyState::Chasing);
                    }
                }
                
                // Check if we've reached the investigation point
                if (AIController && GetActorLocation().Equals(LastKnownPlayerLocation, 100.0f))
                {
                    // Return to default behavior after investigating
                    ReturnToDefaultBehavior();
                }
                break;
                
            default:
                break;
        }
    }
}

// Called to bind functionality to input
void ABaseEnemy::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

float ABaseEnemy::TakeDamageCustom(float DamageAmount, bool bIgnoreInvulnerability)
{
    // If already dead, don't take damage
    if (bIsDead)
    {
        return 0.0f;
    }
    
    // Apply damage to health
    CurrentHealth = FMath::Clamp(CurrentHealth - DamageAmount, 0.0f, MaxHealth);
    
    // Broadcast health changed event
    OnHealthChanged.Broadcast(CurrentHealth, MaxHealth);
    
    // Check if the enemy should die
    if (CurrentHealth <= 0.0f)
    {
        Die();
    }
    
    return DamageAmount;
}

float ABaseEnemy::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
    float ActualDamage = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);
    
    // Call our custom damage handler
    return TakeDamageCustom(ActualDamage);
}

void ABaseEnemy::HealEnemy(float HealAmount)
{
    // Don't heal if dead
    if (bIsDead)
    {
        return;
    }
    
    // Apply healing to health
    CurrentHealth = FMath::Clamp(CurrentHealth + HealAmount, 0.0f, MaxHealth);
    
    // Broadcast health changed event
    OnHealthChanged.Broadcast(CurrentHealth, MaxHealth);
}

void ABaseEnemy::Die()
{
    // Already dead, do nothing
    if (bIsDead)
    {
        return;
    }
    
    // Set dead flag
    bIsDead = true;
    
    // Broadcast death event
    OnEnemyDeath.Broadcast();
    
    // Disable collision
    GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    
    // Disable movement
    GetCharacterMovement()->DisableMovement();
    
    // Clear any active timers
    GetWorldTimerManager().ClearTimer(StunTimerHandle);
    GetWorldTimerManager().ClearTimer(AttackCooldownTimerHandle);
    GetWorldTimerManager().ClearTimer(DeathTimerHandle);
    
    // Update state
    SetEnemyState(EEnemyState::Dead);
    
    // Play death sound
    if (DeathSound && AudioComponent)
    {
        AudioComponent->SetSound(DeathSound);
        AudioComponent->Play();
    }
    
    // Play death animation if available
    if (DeathMontage)
    {
        PlayAnimMontage(DeathMontage);
    }
    
    // Set up cleanup timer
    GetWorldTimerManager().SetTimer(
        DeathTimerHandle,
        [this]()
        {
            // Destroy actor after cleanup time
            Destroy();
        },
        DeathCleanupTime,
        false
    );
}

// Set the enemy state
void ABaseEnemy::SetEnemyState(EEnemyState NewState)
{
    if (CurrentState != NewState)
    {
        EEnemyState PreviousState = CurrentState;
        CurrentState = NewState;
        
        // Handle state-specific setup
        switch (NewState)
        {
            case EEnemyState::Idle:
                GetCharacterMovement()->MaxWalkSpeed = DefaultSpeed;
                // Play idle sound occasionally
                if (FMath::RandBool() && IdleSound && AudioComponent)
                {
                    AudioComponent->SetSound(IdleSound);
                    AudioComponent->Play();
                }
                break;
                
            case EEnemyState::Investigating:
                GetCharacterMovement()->MaxWalkSpeed = InvestigateSpeed;
                // Play investigation sound if available
                if (SpotPlayerSound && AudioComponent)
                {
                    AudioComponent->SetSound(SpotPlayerSound);
                    AudioComponent->Play();
                }
                break;
                
            case EEnemyState::Chasing:
                GetCharacterMovement()->MaxWalkSpeed = ChaseSpeed;
                // Play spot player sound if coming from a non-chase state
                if (PreviousState != EEnemyState::Chasing && PreviousState != EEnemyState::Attacking)
                {
                    if (SpotPlayerSound && AudioComponent)
                    {
                        AudioComponent->SetSound(SpotPlayerSound);
                        AudioComponent->Play();
                    }
                }
                break;
                
            case EEnemyState::Stunned:
                // Stop all movement when stunned
                GetCharacterMovement()->StopMovementImmediately();
                // Play stunned sound
                if (StunnedSound && AudioComponent)
                {
                    AudioComponent->SetSound(StunnedSound);
                    AudioComponent->Play();
                }
                break;
                
            case EEnemyState::Attacking:
                // Handled in PerformAttack method
                break;
                
            case EEnemyState::Dead:
                // Should be handled by Die method
                break;
        }
        
        // Broadcast state change
        OnStateChanged.Broadcast(NewState);
    }
}

// Handle being stunned
void ABaseEnemy::Stun(float Duration)
{
    // Use default duration if no specific duration is provided
    if (Duration < 0.0f)
    {
        Duration = StunDuration;
    }
    
    // Set stunned state
    SetEnemyState(EEnemyState::Stunned);
    
    // Play stun animation if available
    if (StunMontage)
    {
        PlayAnimMontage(StunMontage);
    }
    
    // Clear any existing stun timer
    GetWorldTimerManager().ClearTimer(StunTimerHandle);
    
    // Set timer to end stun after duration
    GetWorldTimerManager().SetTimer(
        StunTimerHandle,
        this,
        &ABaseEnemy::EndStun,
        Duration,
        false
    );
}

// End stun state
void ABaseEnemy::EndStun()
{
    // Return to default behavior if still stunned
    if (CurrentState == EEnemyState::Stunned)
    {
        ReturnToDefaultBehavior();
    }
}

// Perform attack
void ABaseEnemy::PerformAttack()
{
    // Only attack if not on cooldown
    if (!bIsAttackOnCooldown && CurrentState != EEnemyState::Stunned && CurrentState != EEnemyState::Dead)
    {
        SetEnemyState(EEnemyState::Attacking);
        
        // Play attack animation if available
        if (AttackMontage)
        {
            PlayAnimMontage(AttackMontage);
        }
        
        // Play attack sound
        if (AttackSound && AudioComponent)
        {
            AudioComponent->SetSound(AttackSound);
            AudioComponent->Play();
        }
        
        // Try to damage the player
        if (AActor* Player = UGameplayStatics::GetPlayerPawn(this, 0))
        {
            if (IsInAttackRange(Player))
            {
                UGameplayStatics::ApplyDamage(
                    Player,
                    AttackDamage,
                    GetController(),
                    this,
                    UDamageType::StaticClass()
                );
            }
        }
        
        // Start attack cooldown
        StartAttackCooldown();
        
        // Return to chasing after attack
        GetWorldTimerManager().SetTimer(
            AttackCooldownTimerHandle,
            [this]()
            {
                if (CurrentState == EEnemyState::Attacking)
                {
                    SetEnemyState(EEnemyState::Chasing);
                }
            },
            0.5f, // Short delay after attack animation
            false
        );
    }
}

// Start attack cooldown
void ABaseEnemy::StartAttackCooldown()
{
    bIsAttackOnCooldown = true;
    
    // Set timer to end cooldown
    GetWorldTimerManager().SetTimer(
        AttackCooldownTimerHandle,
        [this]()
        {
            bIsAttackOnCooldown = false;
        },
        AttackCooldown,
        false
    );
}

// Check if in attack range of target
bool ABaseEnemy::IsInAttackRange(AActor* Target) const
{
    if (!Target)
    {
        return false;
    }
    
    float Distance = FVector::Dist(GetActorLocation(), Target->GetActorLocation());
    return Distance <= AttackRange;
}

// Move to location
void ABaseEnemy::MoveToLocation(const FVector& Location)
{
    if (AAIController* AIController = Cast<AAIController>(GetController()))
    {
        AIController->MoveToLocation(Location);
    }
}

// React to sound stimulus
void ABaseEnemy::ReactToSound(AActor* SoundSource, const FVector& SoundLocation)
{
    // Only react to sounds if not already chasing or dead or stunned
    if (CurrentState != EEnemyState::Chasing && 
        CurrentState != EEnemyState::Dead && 
        CurrentState != EEnemyState::Stunned)
    {
        // Save the location
        LastKnownPlayerLocation = SoundLocation;
        
        // Move to investigate
        SetEnemyState(EEnemyState::Investigating);
        MoveToLocation(SoundLocation);
    }
}

// React to seeing player
void ABaseEnemy::ReactToSeeingPlayer(APawn* PlayerPawn)
{
    // Don't react if dead or stunned
    if (CurrentState == EEnemyState::Dead || CurrentState == EEnemyState::Stunned)
    {
        return;
    }
    
    // Update last known location
    if (PlayerPawn)
    {
        LastKnownPlayerLocation = PlayerPawn->GetActorLocation();
        
        // Start chasing
        SetEnemyState(EEnemyState::Chasing);
        
        // Broadcast that enemy spotted player
        OnEnemySpotPlayer.Broadcast(PlayerPawn);
        
        // Move to player
        if (AAIController* AIController = Cast<AAIController>(GetController()))
        {
            AIController->MoveToActor(PlayerPawn);
        }
    }
}

// Check line of sight to target
bool ABaseEnemy::HasLineOfSightTo(AActor* Target) const
{
    if (!Target)
    {
        return false;
    }
    
    FHitResult HitResult;
    FVector EyeLocation = GetActorLocation() + FVector(0, 0, BaseEyeHeight);
    FVector TargetLocation = Target->GetActorLocation();
    
    FCollisionQueryParams QueryParams;
    QueryParams.AddIgnoredActor(this);
    
    bool bHit = GetWorld()->LineTraceSingleByChannel(
        HitResult,
        EyeLocation,
        TargetLocation,
        ECC_Visibility,
        QueryParams
    );
    
    return !bHit || HitResult.GetActor() == Target;
}

// Reset to default behavior
void ABaseEnemy::ReturnToDefaultBehavior()
{
    SetEnemyState(EEnemyState::Idle);
    
    // Optionally wander around if idle
    if (FMath::RandBool())
    {
        UNavigationSystemV1* NavSystem = UNavigationSystemV1::GetCurrent(GetWorld());
        if (NavSystem)
        {
            FNavLocation RandomLocation;
            if (NavSystem->GetRandomPointInNavigableRadius(GetActorLocation(), 500.0f, RandomLocation))
            {
                MoveToLocation(RandomLocation.Location);
            }
        }
    }
}

// Handle being hit by flashlight
void ABaseEnemy::ReactToFlashlight(float Intensity)
{
    // Only react if this enemy type is affected by flashlight
    if (bAffectedByFlashlight)
    {
        // Calculate stun chance based on intensity and sensitivity
        float StunChance = FMath::Clamp(Intensity * FlashlightSensitivity / 8000.0f, 0.0f, 0.75f);
        
        // Chance to stun the enemy
        if (FMath::FRand() < StunChance)
        {
            // Stun duration scales with intensity
            float StunTime = FMath::Clamp(StunDuration * (Intensity / 8000.0f), 1.0f, StunDuration);
            Stun(StunTime);
        }
        else if (CurrentState == EEnemyState::Idle)
        {
            // Even if not stunned, high intensity light might make the enemy investigate
            if (Intensity > 4000.0f && FMath::FRand() < 0.5f)
            {
                if (AActor* Player = UGameplayStatics::GetPlayerPawn(this, 0))
                {
                    LastKnownPlayerLocation = GetActorLocation() + (Player->GetActorLocation() - GetActorLocation()).GetSafeNormal() * 300.0f;
                    SetEnemyState(EEnemyState::Investigating);
                    MoveToLocation(LastKnownPlayerLocation);
                }
            }
        }
    }
}

// Called when player is sensed
void ABaseEnemy::OnPlayerSeen(APawn* Pawn)
{
    if (Pawn && Pawn->IsPlayerControlled())
    {
        ReactToSeeingPlayer(Pawn);
    }
}

// Called when a sound is heard
void ABaseEnemy::OnNoiseHeard(APawn* NoiseInstigator, const FVector& Location, float Volume)
{
    // React more strongly to louder sounds
    if (Volume > 0.5f)
    {
        ReactToSound(NoiseInstigator, Location);
    }
}

