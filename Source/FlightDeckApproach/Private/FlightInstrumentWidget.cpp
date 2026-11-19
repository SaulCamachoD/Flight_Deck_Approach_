// Fill out your copyright notice in the Description page of Project Settings.


#include "FlightInstrumentWidget.h"
#include "AircraftPawn.h"
#include "ILSComponent.h"
#include "Components/ProgressBar.h"
#include "ILSTransmitter.h"
#include "TapeWidget.h"
#include "Components/Image.h"
#include "WindSubsystem.h"
#include "LandingScoreComponent.h"
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

	ThrottleText->SetText(FText::FromString(FString::Printf(TEXT("%.0f%%"), Aircraft->GetThrottle() * 100.f)));
	
	// --- Throttle pedido: número arriba y triángulo junto a la barra ---
	const float ThrTarget = Aircraft->GetThrottleTarget();
	if (ThrottleTargetText)
	{
		ThrottleTargetText->SetText(FText::FromString(
			FString::Printf(TEXT("%.0f%%"), ThrTarget * 100.f)));
	}
	if (ThrottleTargetMarker)
	{
		// Sube desde la base de la barra: Y negativa es hacia arriba
		ThrottleTargetMarker->SetRenderTranslation(FVector2D(0.f, -ThrTarget * ThrottleBarHeightPx));
	}

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
		
	// --- Viento ---
	if (const UWindSubsystem* Wind = GetWorld()->GetSubsystem<UWindSubsystem>())
	{
		const int32 BaseKt = FMath::RoundToInt(Wind->GetBaseSpeedKt());
		const int32 PeakKt = FMath::RoundToInt(Wind->GetBaseSpeedKt() + Wind->GetGustKt());

		if (WindText)
		{
			FString S;
			if (PeakKt < 2)
			{
				S = TEXT("CALM");
			}
			else
			{
				// Como el ATIS: dirección en decenas, 0 se dice 360, "G" solo si la ráfaga es notable
				int32 From = FMath::RoundToInt(FRotator::ClampAxis(Wind->GetBaseFromDeg()) / 10.f) * 10;
				if (From == 0) From = 360;
				S = (PeakKt - BaseKt >= 3)
					? FString::Printf(TEXT("%03d° %dG%d KT"), From, BaseKt, PeakKt)
					: FString::Printf(TEXT("%03d° %d KT"), From, BaseKt);
			}
			WindText->SetText(FText::FromString(S));
		}

		float RunwayYaw = 0.f;
		if (ILS && ILS->GetTransmitter())
		{
			RunwayYaw = ILS->GetTransmitter()->GetActorRotation().Yaw;
		}
		const float RelRad = FMath::DegreesToRadians(Wind->GetFromDeg() - RunwayYaw);
		const float Head  = Wind->GetSpeedKt() * FMath::Cos(RelRad);   // + = de frente
		const float Cross = Wind->GetSpeedKt() * FMath::Sin(RelRad);   // + = desde la derecha

		if (CrosswindText)
		{
			CrosswindText->SetText(FText::FromString(FString::Printf(TEXT("XW %.0f KT %s"),
				FMath::Abs(Cross), Cross >= 0.f ? TEXT("R") : TEXT("L"))));
			CrosswindText->SetColorAndOpacity(FMath::Abs(Cross) > 12.f ? HudAmber : HudGreen);
		}
		if (HeadwindText)
		{
			HeadwindText->SetText(FText::FromString(FString::Printf(TEXT("%s %.0f KT"),
				Head >= 0.f ? TEXT("HW") : TEXT("TW"), FMath::Abs(Head))));
			HeadwindText->SetColorAndOpacity(Head < 0.f ? HudAmber : HudGreen);
		}

		// Flecha: hacia dónde sopla, relativo al morro (arriba = hacia delante)
		if (WindArrow)
		{
			WindArrow->SetRenderTransformAngle(Wind->GetFromDeg() + 180.f - Aircraft->GetHeadingDeg());
		}
	}

	// --- Approach score en vivo ---
	if (const ULandingScoreComponent* Score = Aircraft->GetLandingScore())
	{
		const float Pct = Score->GetLiveTrackingPct();
		const FLinearColor C = Pct >= 70.f ? HudGreen : (Pct >= 40.f ? HudAmber : HudRed);

		if (ApproachScoreText)
		{
			ApproachScoreText->SetText(FText::FromString(FString::Printf(TEXT("%.0f%%"), Pct)));
			ApproachScoreText->SetColorAndOpacity(C);
		}
		if (ApproachScoreBar)
		{
			ApproachScoreBar->SetPercent(Pct / 100.f);
			ApproachScoreBar->SetFillColorAndOpacity(C);
		}
	}	
	
	// --- Cintas ---
	if (SpeedTape)    SpeedTape->SetValue(Aircraft->GetAirspeedKnots());
	if (AltitudeTape) AltitudeTape->SetValue(Aircraft->GetAltitudeFeet());
	if (HeadingTape)  HeadingTape->SetValue(Aircraft->GetHeadingDeg());
	
	// --- VSI ---
	const float VSfpm = Aircraft->GetVerticalSpeedFPM();

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
