// Fill out your copyright notice in the Description page of Project Settings.


#include "Characters/AnimInstance/PlayerAnimationInstance.h"
#include "Characters/BaseCharacter.h"
#include "Characters/PlayerCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"

void UPlayerAnimationInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();

	PlayerCharacter = Cast<APlayerCharacter>(TryGetPawnOwner());

	if (PlayerCharacter)
	{
		PlayerCharacterMovement = PlayerCharacter->GetCharacterMovement();
	}
}

void UPlayerAnimationInstance::NativeUpdateAnimation(float DeltaTime)
{
	Super::NativeUpdateAnimation(DeltaTime);
	if (PlayerCharacterMovement)
	{
		isHoldingFlashlight = PlayerCharacter->GetIsHoldingFlashlight();
	}
}
