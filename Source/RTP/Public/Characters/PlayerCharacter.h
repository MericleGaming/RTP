// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BaseCharacter.h"
#include "Blueprint/UserWidget.h"
#include "InputActionValue.h"
#include "PlayerCharacter.generated.h"

class UCameraComponent;
class USkeletalMeshComponent;
class UInputMappingContext;
class UInputAction;
class UStaminaWidget;
class UFlashlightWidget;
class USpotLightComponent;
class UAudioComponent;

// Enum for flashlight modes
UENUM(BlueprintType)
enum class EFlashlightMode : uint8
{
    Off,
    Low,
    Medium,
    High,
    Strobe
};

UCLASS()
class RTP_API APlayerCharacter : public ABaseCharacter
{
	GENERATED_BODY()

public:
	APlayerCharacter();

	virtual void Tick(float DeltaTime) override;

	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

protected:
	virtual void BeginPlay() override;

	void Move(const FInputActionValue& Value);

	void Look(const FInputActionValue& Value);

	virtual void Jump() override;

	void StartSprinting(const FInputActionValue& Value);
	
	void StopSprinting(const FInputActionValue& Value);

	void UpdateStamina(float DeltaTime);

	// Updated flashlight controls
	void ToggleFlashlight(const FInputActionValue& Value);
	void CycleFlashlightMode(const FInputActionValue& Value);
	void UpdateFlashlight(float DeltaTime);
	void SetFlashlightMode(EFlashlightMode NewMode);

private:

	bool bIsSprinting;
	float CurrentStamina;
	float MaxStamina;
	float StaminaRecoveryRate;
	float StaminaConsumptionRate;
	float StaminaRegenDelay;
	float StaminaRegenTimer;
	float StaminaRecoveryBuffer;
	float StaminaConsumptionBuffer;
	float NormalSpeed;
	float SprintSpeed;
		// Flashlight properties
	bool bIsHoldingFlashlight;
	
	// Battery system
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Flashlight", meta = (AllowPrivateAccess = true))
	float MaxBatteryLife = 100.0f;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Flashlight", meta = (AllowPrivateAccess = true))
	float CurrentBatteryLife;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Flashlight", meta = (AllowPrivateAccess = true))
	float BatteryDrainRate = 1.0f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Flashlight", meta = (AllowPrivateAccess = true))
	float BatteryRechargeRate = 0.5f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Flashlight", meta = (AllowPrivateAccess = true))
	float BatteryDrainMultiplierHigh = 2.0f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Flashlight", meta = (AllowPrivateAccess = true))
	float BatteryDrainMultiplierMedium = 1.0f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Flashlight", meta = (AllowPrivateAccess = true))
	float BatteryDrainMultiplierLow = 0.5f;
	
		UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Flashlight", meta = (AllowPrivateAccess = true))
	float BatteryDrainMultiplierStrobe = 1.5f;
	
	// Flashlight mode
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Flashlight", meta = (AllowPrivateAccess = true))
	EFlashlightMode CurrentFlashlightMode = EFlashlightMode::Off;
	
	// Stores the last active mode when flashlight is turned off
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Flashlight", meta = (AllowPrivateAccess = true))
	EFlashlightMode LastUsedFlashlightMode = EFlashlightMode::Off;
	
	// Intensity settings for different modes
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Flashlight", meta = (AllowPrivateAccess = true))
	float FlashlightIntensityHigh = 8000.0f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Flashlight", meta = (AllowPrivateAccess = true))
	float FlashlightIntensityMedium = 4000.0f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Flashlight", meta = (AllowPrivateAccess = true))
	float FlashlightIntensityLow = 2000.0f;
	
	// Strobe effect
	float StrobeTimer = 0.0f;
		UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Flashlight", meta = (AllowPrivateAccess = true))
	float StrobeInterval = 0.2f;
	
	bool bStrobeState = false;
	
	// Low battery warning
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Flashlight", meta = (AllowPrivateAccess = true))
	float LowBatteryThreshold = 20.0f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Flashlight", meta = (AllowPrivateAccess = true))
	float LowBatteryFlickerFrequency = 3.0f;
	
	// Battery level at which the flashlight starts to dim
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Flashlight", meta = (AllowPrivateAccess = true))
	float DimmingStartThreshold = 30.0f;
	
	float LowBatteryFlickerTimer = 0.0f;
	
	// Track if the flashlight is currently flickering
	bool bIsFlickering = false;
	
	// Flashlight audio
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Flashlight", meta = (AllowPrivateAccess = true))
	UAudioComponent* FlashlightSound;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Flashlight", meta = (AllowPrivateAccess = true))
	USoundBase* FlashlightToggleSound;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Flashlight", meta = (AllowPrivateAccess = true))
	USoundBase* FlashlightLowBatterySound;

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "Camera", meta = (AllowPrivateAccess = true))
	UCameraComponent* ViewCamera;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Mesh", meta = (AllowPrivateAccess = true))
	USkeletalMeshComponent* PlayerMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Flashlight", meta = (AllowPrivateAccess = true))
	USpotLightComponent* OuterFlashlight;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Flashlight", meta = (AllowPrivateAccess = true))
	USpotLightComponent* InnerFlashlight;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Input", meta = (AllowPrivateAccess = true))
	UInputMappingContext* MappingContext;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Input", meta = (AllowPrivateAccess = true))
	UInputAction* MovementAction;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Input", meta = (AllowPrivateAccess = true))
	UInputAction* LookAction;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Input", meta = (AllowPrivateAccess = true))
	UInputAction* JumpAction;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Input", meta = (AllowPrivateAccess = true))
	UInputAction* SprintAction;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Input", meta = (AllowPrivateAccess = true))
	UInputAction* FlashlightAction;
	
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Input", meta = (AllowPrivateAccess = true))
	UInputAction* FlashlightModeAction;
	UPROPERTY(EditAnywhere, Category = "UI")
	TSubclassOf<UUserWidget> StaminaWidgetClass;

	UPROPERTY()
	UStaminaWidget* StaminaWidget;
	
	UPROPERTY(EditAnywhere, Category = "UI")
	TSubclassOf<UUserWidget> FlashlightWidgetClass;

	UPROPERTY()
	UFlashlightWidget* FlashlightWidget;

public:
	FORCEINLINE UCameraComponent* GetViewCamera() { return ViewCamera; }
	FORCEINLINE bool GetIsHoldingFlashlight() { return bIsHoldingFlashlight; }
	FORCEINLINE EFlashlightMode GetFlashlightMode() { return CurrentFlashlightMode; }
	FORCEINLINE float GetBatteryPercentage() { return CurrentBatteryLife / MaxBatteryLife; }
};
