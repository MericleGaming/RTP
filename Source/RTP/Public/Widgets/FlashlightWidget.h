// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Characters/PlayerCharacter.h" // Added to access flashlight enums
#include "FlashlightWidget.generated.h"

class UTextBlock;
class UProgressBar;

/**
 * 
 */
UCLASS()
class RTP_API UFlashlightWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	UFUNCTION(BlueprintCallable, Category = "UI")
	void UpdateBatteryPercentage(float BatteryPercent);
	
	UFUNCTION(BlueprintCallable, Category = "UI")
	void UpdateFlashlightMode(EFlashlightMode CurrentMode);

protected:
	UPROPERTY(meta = (BindWidget))
	UProgressBar* BatteryProgressBar;
	
	UPROPERTY(meta = (BindWidget))
	UTextBlock* BatteryText;
	
	UPROPERTY(meta = (BindWidget))
	UTextBlock* FlashlightModeText;
};
