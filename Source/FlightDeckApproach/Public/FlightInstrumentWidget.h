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
	UPROPERTY(meta=(BindWidgetOptional)) UTextBlock* ThrottleTargetText;
	UPROPERTY(meta=(BindWidgetOptional)) class UWidget* ThrottleTargetMarker;
	// Alto en px de ThrottleBar: el triángulo recorre esta distancia
	UPROPERTY(EditAnywhere, Category="HUD|Throttle") float ThrottleBarHeightPx = 200.f;
	UPROPERTY(meta=(BindWidget)) UTextBlock* DMEText;
	
	// Agujas de la cruz ILS (Images en el WBP)
	UPROPERTY(meta=(BindWidget)) class UWidget* LocalizerNeedle;
	UPROPERTY(meta=(BindWidget)) class UWidget* GlideslopeNeedle;
	UPROPERTY(meta=(BindWidgetOptional)) UTextBlock* RunwayText;
	
	// Flight path marker: hacia dónde va el avión
	UPROPERTY(meta=(BindWidgetOptional)) class UWidget* FlightPathMarker;

	// Píxeles por grado del FPM
	UPROPERTY(EditAnywhere, Category="HUD|Attitude") float PxPerDeg = 16.f;
	// --- Horizonte redondo (ADI) ---
	UPROPERTY(meta=(BindWidgetOptional)) class UWidget* AdiLadder;
	UPROPERTY(meta=(BindWidgetOptional)) class UWidget* AdiRollPointer;
	UPROPERTY(EditAnywhere, Category="HUD|Attitude") float AdiPxPerDeg = 6.f;
	
	// --- VSI y avisos ---
	UPROPERTY(meta=(BindWidgetOptional)) class UImage* VSIBar;
	UPROPERTY(meta=(BindWidgetOptional)) UTextBlock* SinkRateText;
	UPROPERTY(meta=(BindWidgetOptional)) UTextBlock* GearText;   
	// --- Cintas ---
	UPROPERTY(meta=(BindWidgetOptional)) class UTapeWidget* SpeedTape;
	UPROPERTY(meta=(BindWidgetOptional)) class UTapeWidget* AltitudeTape;
	UPROPERTY(meta=(BindWidgetOptional)) class UTapeWidget* HeadingTape;
	
	// --- Viento ---
	UPROPERTY(meta=(BindWidgetOptional)) UTextBlock* WindText;        // "270° 12G18 KT"
	UPROPERTY(meta=(BindWidgetOptional)) UTextBlock* CrosswindText;   // "XW 8 KT R"
	UPROPERTY(meta=(BindWidgetOptional)) UTextBlock* HeadwindText;    // "HW 10 KT"
	UPROPERTY(meta=(BindWidgetOptional)) class UWidget* WindArrow;    // flecha apuntando ARRIBA en el WBP

	// --- Approach score en vivo ---
	UPROPERTY(meta=(BindWidgetOptional)) UTextBlock* ApproachScoreText;
	UPROPERTY(meta=(BindWidgetOptional)) class UProgressBar* ApproachScoreBar;

	UPROPERTY(EditAnywhere, Category="HUD|Colors") FLinearColor HudGreen = FLinearColor::FromSRGBColor(FColor(0x7C, 0xFF, 0xB0));
	UPROPERTY(EditAnywhere, Category="HUD|Colors") FLinearColor HudAmber = FLinearColor::FromSRGBColor(FColor(0xFF, 0xA0, 0x40));
	UPROPERTY(EditAnywhere, Category="HUD|Colors") FLinearColor HudRed   = FLinearColor::FromSRGBColor(FColor(0xFF, 0x40, 0x30));
	
	// fpm que llenan la barra entera (hacia arriba o hacia abajo)
	UPROPERTY(EditAnywhere, Category="HUD|VSI") float VSIRangeFPM = 2000.f;
	// Aviso si desciendes más rápido que esto por debajo de SinkRateMaxAltFt
	UPROPERTY(EditAnywhere, Category="HUD|Warnings") float SinkRateWarnFPM = 1000.f;
	UPROPERTY(EditAnywhere, Category="HUD|Warnings") float SinkRateMaxAltFt = 2000.f;
	UPROPERTY(EditAnywhere, Category="HUD|Warnings") float GearWarnAltFt = 1500.f;
	
	
	
	// Píxeles que se mueve la aguja con desviación máxima (±1)
	UPROPERTY(EditAnywhere, Category="HUD|ILS") float NeedleRangePx = 120.f;

private:
	UPROPERTY() TObjectPtr<AAircraftPawn> Aircraft;	
};
