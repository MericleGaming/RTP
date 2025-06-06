// Fill out your copyright notice in the Description page of Project Settings.


#include "Widgets/FlashlightWidget.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"

void UFlashlightWidget::UpdateBatteryPercentage(float BatteryPercent)
{
    // Update battery progress bar
    if (BatteryProgressBar)
    {
        BatteryProgressBar->SetPercent(BatteryPercent);
        
        // Apply gradual color change based on battery level
        // Green (0,1,0) at 100%, Yellow (1,1,0) at 50%, Red (1,0,0) at 0%
        FLinearColor BarColor;
        
        if (BatteryPercent > 0.5f)
        {
            // Lerp from green to yellow as battery decreases from 100% to 50%
            float Ratio = (BatteryPercent - 0.5f) * 2.0f; // Maps 0.5-1.0 to 0.0-1.0
            BarColor = FLinearColor::LerpUsingHSV(FLinearColor(1.0f, 1.0f, 0.0f), FLinearColor(0.0f, 1.0f, 0.0f), Ratio);
        }
        else
        {
            // Lerp from yellow to red as battery decreases from 50% to 0%
            float Ratio = BatteryPercent * 2.0f; // Maps 0.0-0.5 to 0.0-1.0
            BarColor = FLinearColor::LerpUsingHSV(FLinearColor(1.0f, 0.0f, 0.0f), FLinearColor(1.0f, 1.0f, 0.0f), Ratio);
        }
        
        BatteryProgressBar->SetFillColorAndOpacity(BarColor);
    }
    
    // Update battery text
    if (BatteryText)
    {
        FText BatteryTextValue = FText::FromString(FString::Printf(TEXT("Battery: %.0f%%"), BatteryPercent * 100));
        BatteryText->SetText(BatteryTextValue);
    }
}

void UFlashlightWidget::UpdateFlashlightMode(EFlashlightMode CurrentMode)
{
    if (FlashlightModeText)
    {
        FString ModeText;
        switch (CurrentMode)
        {
            case EFlashlightMode::Off:
                ModeText = "OFF";
                break;
            case EFlashlightMode::Low:
                ModeText = "LOW";
                break;
            case EFlashlightMode::Medium:
                ModeText = "MED";
                break;
            case EFlashlightMode::High:
                ModeText = "HIGH";
                break;
            case EFlashlightMode::Strobe:
                ModeText = "STROBE";
                break;
            default:
                ModeText = "UNK";
                break;
        }
        
        FText ModeTextValue = FText::FromString(FString::Printf(TEXT("Mode: %s"), *ModeText));
        FlashlightModeText->SetText(ModeTextValue);
    }
}

