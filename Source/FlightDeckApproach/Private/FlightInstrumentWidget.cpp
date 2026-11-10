// Fill out your copyright notice in the Description page of Project Settings.


#include "FlightInstrumentWidget.h"
#include "AircraftPawn.h"
#include "ILSComponent.h"
#include "Components/ProgressBar.h"
#include "ILSTransmitter.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"

namespace
{
	// Gira con el roll y desplaza con el pitch, perpendicular al horizonte ya girado
	void ApplyAttitude(UWidget* W, float PitchDeg, float RollDeg, float PxPerDegree)
	{
		if (!W) return;
		W->SetRenderTransformAngle(-RollDeg);
		const float Offset = PitchDeg * PxPerDegree;
		const float RollRad = FMath::DegreesToRadians(RollDeg);
		W->SetRenderTranslation(FVector2D(
			Offset * FMath::Sin(RollRad),
			Offset * FMath::Cos(RollRad)));
	}
}

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
	const bool bILS = ILS && ILS->IsSignalValid();

	// --- DME ---
	DMEText->SetText(FText::FromString(bILS
		? FString::Printf(TEXT("%.1f NM"), ILS->GetDistanceNM())
		: FString(TEXT("--- NM"))));

	// --- Cruz ILS: se oculta sin señal, como la bandera de un CDI real ---
	const ESlateVisibility NeedleVis = bILS ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Hidden;
	LocalizerNeedle->SetVisibility(NeedleVis);
	GlideslopeNeedle->SetVisibility(NeedleVis);

	if (bILS)
	{
		// LOC > 0 = estás a la derecha del eje -> la aguja va a la izquierda
		LocalizerNeedle->SetRenderTranslation(
			FVector2D(-ILS->GetLocalizerNormalized() * NeedleRangePx, 0.f));

		// GS > 0 = estás alto -> la aguja baja (en pantalla, Y positiva es hacia abajo)
		GlideslopeNeedle->SetRenderTranslation(
			FVector2D(0.f, ILS->GetGlideslopeNormalized() * NeedleRangePx));
	}

	// --- RWY: rumbo 090 -> "09" ---
	if (RunwayText && ILS && ILS->GetTransmitter())
	{
		int32 Rwy = FMath::RoundToInt(ILS->GetTransmitter()->RunwayHeadingDeg / 10.f);
		if (Rwy <= 0) Rwy += 36;   // rumbo 0 o 360 es pista 36
		RunwayText->SetText(FText::FromString(FString::Printf(TEXT("%02d"), Rwy)));
	}
	
	// --- Horizonte artificial (ADI) ---
	const float Pitch = Aircraft->GetPitchDeg();
	const float Roll  = Aircraft->GetRollDeg();

	ApplyAttitude(AdiLadder, Pitch, Roll, AdiPxPerDeg);

	// El indicador de alabeo solo gira, no sube ni baja
	if (AdiRollPointer) AdiRollPointer->SetRenderTransformAngle(-Roll);

	// --- Flight Path Marker ---
	if (FlightPathMarker)
	{
		// Velocidad en ejes del avión: X adelante, Y derecha, Z arriba
		const FVector V = Aircraft->GetActorTransform().InverseTransformVectorNoScale(Aircraft->GetVelocity());
		if (V.X > 100.f)   // solo si avanzamos (> 1 m/s)
		{
			const float VertDeg = FMath::RadiansToDegrees(FMath::Atan2(V.Z, V.X));
			const float LatDeg  = FMath::RadiansToDegrees(FMath::Atan2(V.Y, V.X));
			// Pantalla: Y positiva es hacia abajo, así que la vertical va negada
			FlightPathMarker->SetRenderTranslation(FVector2D(LatDeg * PxPerDeg, -VertDeg * PxPerDeg));
			FlightPathMarker->SetVisibility(ESlateVisibility::HitTestInvisible);
		}
		else
		{
			FlightPathMarker->SetVisibility(ESlateVisibility::Hidden);
		}
	}
	
	// --- VSI ---
	const float VSfpm = Aircraft->GetVerticalSpeedFPM();
	const FLinearColor HudGreen = FLinearColor::FromSRGBColor(FColor(0x7C, 0xFF, 0xB0));
	const FLinearColor HudAmber = FLinearColor::FromSRGBColor(FColor(0xFF, 0xA0, 0x40));

	// --- SINK RATE: solo en el aire, cerca del suelo y bajando rápido ---
	const bool bSink = Aircraft->State == EAircraftState::Airborne
		&& Aircraft->GetAltitudeFeet() < SinkRateMaxAltFt
		&& VSfpm < -SinkRateWarnFPM;

	if (VSIBar)
	{
		// Pivote arriba (0.5, 0): escala Y positiva crece hacia abajo (descenso),
		// escala negativa se voltea hacia arriba (ascenso)
		const float Fill = FMath::Clamp(-VSfpm / VSIRangeFPM, -1.f, 1.f);
		VSIBar->SetRenderScale(FVector2D(1.f, Fill));
		VSIBar->SetColorAndOpacity(bSink ? HudAmber : HudGreen);
	}

	if (SinkRateText)
	{
		// Parpadeo: 0.4 s encendido, 0.2 s apagado
		const bool bBlinkOn = FMath::Fmod(GetWorld()->GetTimeSeconds(), 0.6f) < 0.4f;
		SinkRateText->SetVisibility(bSink && bBlinkOn
			? ESlateVisibility::HitTestInvisible
			: ESlateVisibility::Hidden);
	}
	
}
