// Fill out your copyright notice in the Description page of Project Settings.

#include "Characters/PlayerCharacter.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/SceneComponent.h"
#include "Components/AudioComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Blueprint/UserWidget.h"
#include "Widgets/StaminaWidget.h"
#include "Widgets/FlashlightWidget.h"
#include "Components/SpotLightComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundBase.h"

APlayerCharacter::APlayerCharacter()
{
    PrimaryActorTick.bCanEverTick = true;

    ViewCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("ViewCamera"));
    ViewCamera->SetupAttachment(GetCapsuleComponent());
    ViewCamera->SetRelativeLocation(FVector(-10.f, 0.f, 60.f));
    ViewCamera->bUsePawnControlRotation = true;

    PlayerMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("PlayerMesh"));
    PlayerMesh->SetOnlyOwnerSee(true);
    PlayerMesh->SetupAttachment(ViewCamera);
    PlayerMesh->bCastDynamicShadow = false;
    PlayerMesh->CastShadow = false;
    PlayerMesh->SetRelativeLocation(FVector(-30.f, 0.f, -150.f));

	OuterFlashlight = CreateDefaultSubobject<USpotLightComponent>(TEXT("OuterFlashlight"));
    OuterFlashlight->SetupAttachment(ViewCamera);
    OuterFlashlight->SetRelativeLocation(FVector(-10.f, 0.f, 30.f));
    OuterFlashlight->AttenuationRadius = 10000.f;
    OuterFlashlight->InnerConeAngle = 0.f;
	OuterFlashlight->OuterConeAngle = 45.f;
    OuterFlashlight->SetVisibility(false);

    InnerFlashlight = CreateDefaultSubobject<USpotLightComponent>(TEXT("InnerFlashlight"));
    InnerFlashlight->SetupAttachment(ViewCamera);
    InnerFlashlight->SetRelativeLocation(FVector(-10.f, 0.f, 30.f));
	InnerFlashlight->Intensity = 8000.0f;
	InnerFlashlight->AttenuationRadius = 10000.f;
	InnerFlashlight->InnerConeAngle = 0.f;
	InnerFlashlight->OuterConeAngle = 45.f/2.f;
    InnerFlashlight->SetVisibility(false);
    
    // Initialize audio component for flashlight
    FlashlightSound = CreateDefaultSubobject<UAudioComponent>(TEXT("FlashlightSound"));
    FlashlightSound->SetupAttachment(ViewCamera);
    FlashlightSound->bAutoActivate = false;

    bIsSprinting = false;
    MaxStamina = 100.0f;
    CurrentStamina = MaxStamina;
    StaminaRecoveryRate = 5.0f;
    StaminaRegenDelay = 2.0f;
    StaminaRegenTimer = 0.0f;
    StaminaRecoveryBuffer = 0.01f;
    StaminaConsumptionRate = 10.0f;
	StaminaConsumptionBuffer = 0.3f;

    NormalSpeed = 450.0f;
    SprintSpeed = 900.0f;
    GetCharacterMovement()->MaxWalkSpeed = NormalSpeed;    MaxHealth = 100.0f;
    CurrentHealth = 100.0f;
    HealthRecoveryTimer = 3.0f;
    HealthRecoveryRate = 5.0f;
    
    // Initialize flashlight properties
    bIsHoldingFlashlight = false;
    CurrentBatteryLife = MaxBatteryLife;
    CurrentFlashlightMode = EFlashlightMode::Off;
    LastUsedFlashlightMode = EFlashlightMode::Off;
    bIsFlickering = false;
}

void APlayerCharacter::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    if (GetWorld())
    {
        UpdateStamina(DeltaTime);
        UpdateFlashlight(DeltaTime);

    if (StaminaWidget)
    {
        StaminaWidget->UpdateStaminaBar(CurrentStamina / MaxStamina);
    }
    
    if (FlashlightWidget)
    {
        // Update battery UI
        float BatteryPercent = CurrentBatteryLife / MaxBatteryLife;
        FlashlightWidget->UpdateBatteryPercentage(BatteryPercent);
        
        // Update flashlight mode UI
        FlashlightWidget->UpdateFlashlightMode(CurrentFlashlightMode);
    }
    }
}

void APlayerCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
    Super::SetupPlayerInputComponent(PlayerInputComponent);

    if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent))
    {
        EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &ThisClass::Jump);
        EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);
        EnhancedInputComponent->BindAction(MovementAction, ETriggerEvent::Triggered, this, &ThisClass::Move);
        EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &ThisClass::Look);
        EnhancedInputComponent->BindAction(SprintAction, ETriggerEvent::Started, this, &ThisClass::StartSprinting);
        EnhancedInputComponent->BindAction(SprintAction, ETriggerEvent::Completed, this, &ThisClass::StopSprinting);
        
        // Updated flashlight controls
		EnhancedInputComponent->BindAction(FlashlightAction, ETriggerEvent::Triggered, this, &ThisClass::ToggleFlashlight);
		
		// Only bind mode cycling if the action is set
		if (FlashlightModeAction)
		{
		    EnhancedInputComponent->BindAction(FlashlightModeAction, ETriggerEvent::Triggered, this, &ThisClass::CycleFlashlightMode);
		}
    }
}

void APlayerCharacter::BeginPlay()
{
    Super::BeginPlay();

    if (APlayerController* PlayerController = Cast<APlayerController>(Controller))
    {        if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
        {
            Subsystem->AddMappingContext(MappingContext, 0);
        }
    }
    
    if (StaminaWidgetClass)
    {
        StaminaWidget = CreateWidget<UStaminaWidget>(GetWorld(), StaminaWidgetClass);
        if (StaminaWidget)
        {
            StaminaWidget->AddToViewport();
        }
    }
    
    if (FlashlightWidgetClass)
    {
        FlashlightWidget = CreateWidget<UFlashlightWidget>(GetWorld(), FlashlightWidgetClass);
        if (FlashlightWidget)
        {
            FlashlightWidget->AddToViewport();
        }
    }
    
    // Initialize battery life
    CurrentBatteryLife = MaxBatteryLife;
}

void APlayerCharacter::Move(const FInputActionValue& Value)
{
    FVector2D MovementVector = Value.Get<FVector2D>();

    if (Controller)
    {
        AddMovementInput(GetActorForwardVector(), MovementVector.Y);
        AddMovementInput(GetActorRightVector(), MovementVector.X);
    }
}

void APlayerCharacter::Look(const FInputActionValue& Value)
{
    FVector2D LookAxisVector = Value.Get<FVector2D>();

    if (Controller)
    {
        AddControllerYawInput(LookAxisVector.X);
        AddControllerPitchInput(LookAxisVector.Y);
    }
}

void APlayerCharacter::Jump()
{
    Super::Jump();
}

void APlayerCharacter::StartSprinting(const FInputActionValue& Value)
{
    if (CurrentStamina / MaxStamina > StaminaRecoveryBuffer && CurrentStamina / MaxStamina > StaminaConsumptionBuffer)
    {
        bIsSprinting = true;
        StaminaRegenTimer = 0.0f;
        GetCharacterMovement()->MaxWalkSpeed = SprintSpeed;
    }
}

void APlayerCharacter::StopSprinting(const FInputActionValue& Value)
{
    bIsSprinting = false;
    StaminaRegenTimer = 0.0f;
    GetCharacterMovement()->MaxWalkSpeed = NormalSpeed;
}

void APlayerCharacter::UpdateStamina(float DeltaTime)
{
    if (bIsSprinting)
    {
        CurrentStamina -= StaminaConsumptionRate * DeltaTime;
        if (CurrentStamina <= 0.0f)
        {
            CurrentStamina = 0.0f;
            bIsSprinting = false;
            GetCharacterMovement()->MaxWalkSpeed = NormalSpeed;
        }
    }
    else
    {
        if (StaminaRegenTimer < StaminaRegenDelay && CurrentStamina / MaxStamina < StaminaRecoveryBuffer)
        {
            StaminaRegenTimer += DeltaTime;
        }
        else
        {
            CurrentStamina += StaminaRecoveryRate * DeltaTime;
            if (CurrentStamina > MaxStamina)
            {
                CurrentStamina = MaxStamina;
            }
        }
    }
}

void APlayerCharacter::ToggleFlashlight(const FInputActionValue& Value)
{
    // If we have enough battery, toggle the flashlight
    if (CurrentFlashlightMode == EFlashlightMode::Off && CurrentBatteryLife > 0.0f)
    {
        // Turn on the flashlight to the last used mode or Medium mode if it's the first time
        // We need to store the last mode when turning off
        if (LastUsedFlashlightMode == EFlashlightMode::Off)
        {
            SetFlashlightMode(EFlashlightMode::Medium);
        }
        else
        {
            SetFlashlightMode(LastUsedFlashlightMode);
        }
        bIsHoldingFlashlight = true;
    }
    else
    {
        // Remember the current mode before turning off
        LastUsedFlashlightMode = CurrentFlashlightMode;
        
        // Turn off the flashlight
        SetFlashlightMode(EFlashlightMode::Off);
        bIsHoldingFlashlight = false;
    }
    
    // Play toggle sound
    if (FlashlightToggleSound)
    {
        UGameplayStatics::PlaySoundAtLocation(this, FlashlightToggleSound, GetActorLocation());
    }
}

void APlayerCharacter::CycleFlashlightMode(const FInputActionValue& Value)
{
    // Only cycle if the flashlight is on and has battery
    if (bIsHoldingFlashlight && CurrentBatteryLife > 0.0f)
    {
        // Cycle through modes
        switch (CurrentFlashlightMode)
        {
            case EFlashlightMode::Low:
                SetFlashlightMode(EFlashlightMode::Medium);
                break;
            case EFlashlightMode::Medium:
                SetFlashlightMode(EFlashlightMode::High);
                break;
            case EFlashlightMode::High:
                SetFlashlightMode(EFlashlightMode::Strobe);
                break;
            case EFlashlightMode::Strobe:
                SetFlashlightMode(EFlashlightMode::Low);
                break;
            default:
                SetFlashlightMode(EFlashlightMode::Medium);
                break;
        }
        
        // Play toggle sound at a lower volume for mode changes
        if (FlashlightToggleSound)
        {
            UGameplayStatics::PlaySoundAtLocation(this, FlashlightToggleSound, GetActorLocation(), 0.5f);
        }
    }
}

void APlayerCharacter::UpdateFlashlight(float DeltaTime)
{
    // Only update if flashlight is on
    if (CurrentFlashlightMode != EFlashlightMode::Off)
    {
        // Calculate battery drain based on current mode
        float DrainMultiplier = 0.0f;
        switch (CurrentFlashlightMode)
        {
            case EFlashlightMode::Low:
                DrainMultiplier = BatteryDrainMultiplierLow;
                break;
            case EFlashlightMode::Medium:
                DrainMultiplier = BatteryDrainMultiplierMedium;
                break;
            case EFlashlightMode::High:
                DrainMultiplier = BatteryDrainMultiplierHigh;
                break;
            case EFlashlightMode::Strobe:
                DrainMultiplier = BatteryDrainMultiplierStrobe;
                break;
            default:
                DrainMultiplier = 0.0f;
                break;
        }
          // Drain battery
        CurrentBatteryLife -= BatteryDrainRate * DrainMultiplier * DeltaTime;
        
        // Handle low battery effects - continue flickering until completely depleted
        if (CurrentBatteryLife <= LowBatteryThreshold)
        {
            // Update low battery flicker - intensity increases as battery depletes
            LowBatteryFlickerTimer += DeltaTime;
            if (LowBatteryFlickerTimer >= 1.0f / LowBatteryFlickerFrequency)
            {
                LowBatteryFlickerTimer = 0.0f;
                  // Random chance to flicker based on battery percentage
                // Higher chance to flicker as battery depletes
                float FlickerChance = 1.0f - (CurrentBatteryLife / LowBatteryThreshold);
                
                // Ensure flickering still occurs at 0% battery
                if (CurrentBatteryLife <= 5.0f) // Very low battery threshold
                {
                    FlickerChance = 0.9f; // Very high chance to flicker, but not constant
                }
                
                if (FMath::FRand() < FlickerChance)
                {
                    // More dramatic flickering for very low battery
                    float FlickerIntensity = 0.5f;                    if (CurrentBatteryLife < 5.0f)
                    {
                        // More severe flickering (dimmer) when almost depleted
                        FlickerIntensity = 0.2f;
                    }
                    
                    // Temporarily dim the lights
                    InnerFlashlight->SetIntensity(InnerFlashlight->Intensity * FlickerIntensity);
                    OuterFlashlight->SetIntensity(OuterFlashlight->Intensity * FlickerIntensity);
                    
                    // Set the flickering flag
                    bIsFlickering = true;
                    
                    // Schedule restoration to normal brightness
                    FTimerHandle FlickerRestoreTimerHandle;
                    GetWorldTimerManager().SetTimer(
                        FlickerRestoreTimerHandle,
                        [this]()
                        {
                            // Reset flickering flag
                            bIsFlickering = false;
                            
                            // Restore original intensity based on current mode
                            float TargetIntensity = 0.0f;
                            switch (CurrentFlashlightMode)
                            {
                                case EFlashlightMode::Low:
                                    TargetIntensity = FlashlightIntensityLow;
                                    break;
                                case EFlashlightMode::Medium:
                                    TargetIntensity = FlashlightIntensityMedium;
                                    break;
                                case EFlashlightMode::High:
                                    TargetIntensity = FlashlightIntensityHigh;
                                    break;
                                default:
                                    break;
                            }
                              if (CurrentFlashlightMode != EFlashlightMode::Off && CurrentFlashlightMode != EFlashlightMode::Strobe)
                            {
                                // Apply gradual dimming based on battery percentage
                                if (CurrentBatteryLife < DimmingStartThreshold)
                                {
                                    // Calculate dimming factor (from 0.1 at 0% to 1.0 at DimmingStartThreshold%)
                                    float DimFactor = FMath::Max(0.1f, CurrentBatteryLife / DimmingStartThreshold);
                                    TargetIntensity *= DimFactor;
                                }
                                
                                InnerFlashlight->SetIntensity(TargetIntensity);
                                OuterFlashlight->SetIntensity(TargetIntensity * 0.5f);
                            }                },
                        // Longer flicker duration for nearly depleted battery
                        (CurrentBatteryLife < 5.0f) ? 0.2f : 0.1f,
                        false
                    );
                }
            }
        }
        
        // Apply gradual dimming effect based on battery level
        if (CurrentFlashlightMode != EFlashlightMode::Off && CurrentFlashlightMode != EFlashlightMode::Strobe && !bIsFlickering)
        {
            float CurrentIntensity = 0.0f;
            switch (CurrentFlashlightMode)
            {
                case EFlashlightMode::Low:
                    CurrentIntensity = FlashlightIntensityLow;
                    break;
                case EFlashlightMode::Medium:
                    CurrentIntensity = FlashlightIntensityMedium;
                    break;
                case EFlashlightMode::High:
                    CurrentIntensity = FlashlightIntensityHigh;
                    break;
                default:
                    break;
            }
            
            // Start dimming gradually based on DimmingStartThreshold
            if (CurrentBatteryLife < DimmingStartThreshold)
            {
                // Calculate dimming factor (from 0.1 at 0% to 1.0 at DimmingStartThreshold%)
                float DimFactor = FMath::Max(0.1f, CurrentBatteryLife / DimmingStartThreshold);
                
                // Apply dimming                InnerFlashlight->SetIntensity(CurrentIntensity * DimFactor);
                OuterFlashlight->SetIntensity(CurrentIntensity * 0.5f * DimFactor);
            }
        }
        
        // Handle strobe effect
        if (CurrentFlashlightMode == EFlashlightMode::Strobe)
        {
            StrobeTimer += DeltaTime;
            if (StrobeTimer >= StrobeInterval)
            {
                StrobeTimer = 0.0f;
                bStrobeState = !bStrobeState;
                
                InnerFlashlight->SetVisibility(bStrobeState);
                OuterFlashlight->SetVisibility(bStrobeState);
            }
        }
        
        // If battery is depleted, turn off the flashlight
        if (CurrentBatteryLife <= 0.0f)
        {
            CurrentBatteryLife = 0.0f;
            SetFlashlightMode(EFlashlightMode::Off);
            bIsHoldingFlashlight = false;
        }
    }
    else
    {
        // Recharge battery when flashlight is off
        if (CurrentBatteryLife < MaxBatteryLife)
        {
            CurrentBatteryLife += BatteryRechargeRate * DeltaTime;
            if (CurrentBatteryLife > MaxBatteryLife)
            {
                CurrentBatteryLife = MaxBatteryLife;
            }
        }
    }
}

void APlayerCharacter::SetFlashlightMode(EFlashlightMode NewMode)
{
    CurrentFlashlightMode = NewMode;
      // Reset strobe variables if changing modes
    StrobeTimer = 0.0f;
    bStrobeState = true;
    
    // Update flashlight settings based on mode    
    switch (NewMode)
    {
        case EFlashlightMode::Off:
            InnerFlashlight->SetVisibility(false);
            OuterFlashlight->SetVisibility(false);
            bIsFlickering = false; // Reset flickering flag when turning off
            break;
            
        case EFlashlightMode::Low:
        {            InnerFlashlight->SetVisibility(true);
            OuterFlashlight->SetVisibility(true);
              
            float TargetIntensity = FlashlightIntensityLow;
            // Apply gradual dimming effect based on battery level
            if (CurrentBatteryLife < DimmingStartThreshold)
            {
                // Calculate dimming factor (from 0.1 at 0% to 1.0 at DimmingStartThreshold%)
                float DimFactor = FMath::Max(0.1f, CurrentBatteryLife / DimmingStartThreshold);
                TargetIntensity *= DimFactor;
            }
            
            InnerFlashlight->SetIntensity(TargetIntensity);
            OuterFlashlight->SetIntensity(TargetIntensity * 0.5f);
            OuterFlashlight->OuterConeAngle = 40.0f;
            InnerFlashlight->OuterConeAngle = 25.0f;
            break;        }
            
        case EFlashlightMode::Medium:
        {
            InnerFlashlight->SetVisibility(true);
            OuterFlashlight->SetVisibility(true);
              
            float TargetIntensity = FlashlightIntensityMedium;
            // Apply gradual dimming effect based on battery level
            if (CurrentBatteryLife < DimmingStartThreshold)
            {
                // Calculate dimming factor (from 0.1 at 0% to 1.0 at DimmingStartThreshold%)
                float DimFactor = FMath::Max(0.1f, CurrentBatteryLife / DimmingStartThreshold);
                TargetIntensity *= DimFactor;
            }
            
            InnerFlashlight->SetIntensity(TargetIntensity);
            OuterFlashlight->SetIntensity(TargetIntensity * 0.5f);
            OuterFlashlight->OuterConeAngle = 45.0f;
            InnerFlashlight->OuterConeAngle = 45.0f/2.0f;
            break;        }
            
        case EFlashlightMode::High:
        {
            InnerFlashlight->SetVisibility(true);
            OuterFlashlight->SetVisibility(true);
              
            float TargetIntensity = FlashlightIntensityHigh;
            // Apply gradual dimming effect based on battery level
            if (CurrentBatteryLife < DimmingStartThreshold)
            {
                // Calculate dimming factor (from 0.1 at 0% to 1.0 at DimmingStartThreshold%)
                float DimFactor = FMath::Max(0.1f, CurrentBatteryLife / DimmingStartThreshold);
                TargetIntensity *= DimFactor;
            }
            
            InnerFlashlight->SetIntensity(TargetIntensity);
            OuterFlashlight->SetIntensity(TargetIntensity * 0.5f);
            OuterFlashlight->OuterConeAngle = 50.0f;
            InnerFlashlight->OuterConeAngle = 30.0f;
            break;
        }
            
        case EFlashlightMode::Strobe:
            InnerFlashlight->SetVisibility(true);
            OuterFlashlight->SetVisibility(true);
            InnerFlashlight->SetIntensity(FlashlightIntensityHigh * 1.2f); // Extra bright for strobe
            OuterFlashlight->SetIntensity(FlashlightIntensityHigh * 0.6f);
            OuterFlashlight->OuterConeAngle = 45.0f;
            InnerFlashlight->OuterConeAngle = 30.0f;
            break;
    }
}
