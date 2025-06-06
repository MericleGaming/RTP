// Fill out your copyright notice in the Description page of Project Settings.


#include "Widgets/StaminaWidget.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"

void UStaminaWidget::UpdateStaminaBar(float StaminaPercent)
{
	if (StaminaProgressBar)
	{
		StaminaProgressBar->SetPercent(StaminaPercent);
	}

	if (StaminaText)
	{
		FText StaminaTextValue = FText::FromString(FString::Printf(TEXT("Stamina: %.0f%%"), StaminaPercent * 100));

		StaminaText->SetText(StaminaTextValue);
	}
}
