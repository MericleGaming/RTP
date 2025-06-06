// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "StaminaWidget.generated.h"

/**
 * 
 */
UCLASS()
class RTP_API UStaminaWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	UFUNCTION(BlueprintCallable, Category = "UI")
	void UpdateStaminaBar(float StaminaPercent);

protected:
	UPROPERTY(meta = (BindWidget))
	class UProgressBar* StaminaProgressBar;

	UPROPERTY(meta = (BindWidget))
	class UTextBlock* StaminaText;
};
