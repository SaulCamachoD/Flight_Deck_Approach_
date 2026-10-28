// Fill out your copyright notice in the Description page of Project Settings.


#include "FlightInstrumentWidget.h"
#include "AircraftPawn.h"
#include "ILSComponent.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"


void UFlightInstrumentWidget::NativeConstruct()
{
	Super::NativeConstruct();
	Aircraft = Cast<AAircraftPawn>(GetOwningPlayerPawn());
}

void UFlightInstrumentWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);
	
	if (!Aircraft)
	{
		Aircraft = Cast<AAircraftPawn>(GetOwningPlayerPawn());
		if (!Aircraft) return;
	}

	AirspeedText->SetText(FText::FromString(
		FString::Printf(TEXT("%03.0f KT"), Aircraft->GetAirspeedKnots())));

	AltitudeText->SetText(FText::Format(INVTEXT("{0} FT"),
	FText::AsNumber(FMath::RoundToInt(Aircraft->GetAltitudeFeet()))));

	// El VSI real redondea a decenas: así el número no "tiembla"
	const int32 VS = FMath::RoundToInt(Aircraft->GetVerticalSpeedFPM() / 10.f) * 10;
	VerticalSpeedText->SetText(FText::FromString(
		FString::Printf(TEXT("%+d FPM"), VS)));

	if (ThrottleBar) ThrottleBar->SetPercent(Aircraft->GetThrottle());

	ThrottleText->SetText(FText::FromString(
	   FString::Printf(TEXT("%.0f%%"), Aircraft->GetThrottle() * 100.f)));

	const UILSComponent* ILS = Aircraft->GetILS();
	DMEText->SetText(FText::FromString(
		(ILS && ILS->IsSignalValid())
		? FString::Printf(TEXT("%.1f NM"), ILS->GetDistanceNM())
		: FString(TEXT("--- NM"))));
	
}
