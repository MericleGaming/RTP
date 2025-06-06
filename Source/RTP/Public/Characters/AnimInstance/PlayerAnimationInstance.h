// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "PlayerAnimationInstance.generated.h"

class APlayerCharacter;
class UCharacterMovementComponent;

/**
 * 
 */
UCLASS()
class RTP_API UPlayerAnimationInstance : public UAnimInstance
{
	GENERATED_BODY()
	
public:
	virtual void NativeInitializeAnimation() override;

	virtual void NativeUpdateAnimation(float DeltaTime) override;

	UPROPERTY(BlueprintReadOnly)
	APlayerCharacter* PlayerCharacter;

	UPROPERTY(BlueprintReadOnly, Category="Movement")
	UCharacterMovementComponent* PlayerCharacterMovement;

	UPROPERTY(BlueprintReadOnly, Category="Flashlight")
	bool isHoldingFlashlight;
};
