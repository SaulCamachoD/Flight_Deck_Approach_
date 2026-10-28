// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "FlightInstrumentWidget.generated.h"

class UTextBlock;
class AAircraftPawn;
UCLASS()
class FLIGHTDECKAPPROACH_API UFlightInstrumentWidget : public UUserWidget
{
	GENERATED_BODY()
	
protected:
	virtual void NativeConstruct() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

	// Deben existir en el WBP con estos nombres EXACTOS
	UPROPERTY(meta=(BindWidget)) UTextBlock* AirspeedText;
	UPROPERTY(meta=(BindWidget)) UTextBlock* AltitudeText;
	UPROPERTY(meta=(BindWidget)) UTextBlock* VerticalSpeedText;
	UPROPERTY(meta=(BindWidget)) UTextBlock* ThrottleText;
	UPROPERTY(meta=(BindWidgetOptional)) class UProgressBar* ThrottleBar;
	UPROPERTY(meta=(BindWidget)) UTextBlock* DMEText;

private:
	UPROPERTY() TObjectPtr<AAircraftPawn> Aircraft;	
};
