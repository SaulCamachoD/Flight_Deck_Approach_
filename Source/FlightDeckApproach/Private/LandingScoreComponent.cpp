// Fill out your copyright notice in the Description page of Project Settings.


#include "LandingScoreComponent.h"
#include "ILSComponent.h"
#include "ILSTransmitter.h"
#include "Engine/Engine.h"


ULandingScoreComponent::ULandingScoreComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void ULandingScoreComponent::BeginPlay()
{
	Super::BeginPlay();

	Pawn = Cast<AAircraftPawn>(GetOwner());
	if (!Pawn)
	{
		UE_LOG(LogTemp, Warning, TEXT("LandingScoreComponent: el dueño no es un AAircraftPawn"));
		return;
	}

	// Escuchamos los eventos que el avión ya emite
	Pawn->OnTouchdown.AddDynamic(this, &ULandingScoreComponent::HandleTouchdown);
	Pawn->OnCrash.AddDynamic(this, &ULandingScoreComponent::HandleCrash);
	Pawn->OnStopped.AddDynamic(this, &ULandingScoreComponent::HandleStopped);
}

void ULandingScoreComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// Solo medimos en vuelo, con señal ILS y lejos del umbral
	if (!Pawn || Pawn->State != EAircraftState::Airborne) return;
	const UILSComponent* ILS = Pawn->GetILS();
	if (!ILS || !ILS->IsSignalValid() || ILS->GetDistanceNM() < MinSampleDistanceNM) return;

	// Promedio ponderado por tiempo: así no depende de los FPS
	SumLocDev  += FMath::Abs(ILS->GetLocalizerNormalized())  * DeltaTime;
	SumGsDev   += FMath::Abs(ILS->GetGlideslopeNormalized()) * DeltaTime;
	SampleTime += DeltaTime;
}

float ULandingScoreComponent::GetLiveTrackingPct() const
{
	if (SampleTime <= 0.f) return 100.f;
	const float AvgDev = (SumLocDev + SumGsDev) / (2.f * SampleTime);
	return 100.f * (1.f - AvgDev);
}

void ULandingScoreComponent::HandleTouchdown(const FTouchdownData& Data)
{
	Result.SinkRateFPM      = -Data.VerticalSpeedFPM;   // positivo = bajando
	Result.TouchdownSpeedKt = Data.AirspeedKnots;

	// Punto de toque en el espacio de la pista (el del transmisor, en el umbral):
	// X = cm pasado el umbral, Y = cm a la derecha del eje
	const UILSComponent* ILS = Pawn->GetILS();
	if (const AILSTransmitter* T = ILS ? ILS->GetTransmitter() : nullptr)
	{
		const FVector Local = T->GetActorTransform().InverseTransformPosition(Data.Location);
		Result.DistanceFromAimPointM = Local.X * 0.01f - T->AimPointMetersFromThreshold;
		Result.CenterlineOffsetM     = Local.Y * 0.01f;
	}
}

void ULandingScoreComponent::HandleCrash(ECrashReason Reason)
{
	Result.bCrashed = true;
	Result.CrashReason = Reason;
	Finish();
}

void ULandingScoreComponent::HandleStopped()
{
	Finish();
}

float ULandingScoreComponent::Score01(float Value, float FullAt, float ZeroAt)
{
	// 1 hasta FullAt, baja en línea recta y llega a 0 en ZeroAt
	return 1.f - FMath::Clamp((Value - FullAt) / (ZeroAt - FullAt), 0.f, 1.f);
}

FString ULandingScoreComponent::GradeFor(int32 Score)
{
	if (Score >= 900) return TEXT("EXCELLENT");
	if (Score >= 750) return TEXT("GOOD");
	if (Score >= 500) return TEXT("FAIR");
	return TEXT("POOR");
}

void ULandingScoreComponent::Finish()
{
	if (bHasResult) return;   // solo una vez por intento
	bHasResult = true;
	SetComponentTickEnabled(false);

	if (SampleTime > 0.f)
	{
		Result.LocalizerTrackingPct  = 100.f * (1.f - SumLocDev / SampleTime);
		Result.GlideslopeTrackingPct = 100.f * (1.f - SumGsDev  / SampleTime);
	}

	if (Result.bCrashed)
	{
		Result.TotalScore = 0;
		Result.Grade = TEXT("CRASH");
	}
	else
	{
		Result.TrackingRatio   = (Result.LocalizerTrackingPct + Result.GlideslopeTrackingPct) / 200.f;
		Result.SinkRatio       = Score01(Result.SinkRateFPM, SinkFullFPM, SinkZeroFPM);
		Result.ZoneRatio       = Score01(FMath::Abs(Result.DistanceFromAimPointM), ZoneFullM, ZoneZeroM);
		Result.CenterlineRatio = Score01(FMath::Abs(Result.CenterlineOffsetM), CenterFullM, CenterZeroM);
		Result.SpeedRatio      = Score01(FMath::Abs(Result.TouchdownSpeedKt - TargetTouchdownKt), SpeedFullKt, SpeedZeroKt);

		Result.TrackingPoints   = FMath::RoundToInt(MaxTrackingPoints   * Result.TrackingRatio);
		Result.SinkRatePoints   = FMath::RoundToInt(MaxSinkRatePoints   * Result.SinkRatio);
		Result.ZonePoints       = FMath::RoundToInt(MaxZonePoints       * Result.ZoneRatio);
		Result.CenterlinePoints = FMath::RoundToInt(MaxCenterlinePoints * Result.CenterlineRatio);
		Result.SpeedPoints      = FMath::RoundToInt(MaxSpeedPoints      * Result.SpeedRatio);

		Result.TotalScore = Result.TrackingPoints + Result.SinkRatePoints + Result.ZonePoints
						  + Result.CenterlinePoints + Result.SpeedPoints;
		Result.Grade = GradeFor(Result.TotalScore);
	}

	// Temporal hasta que exista el Landing Report (Parte 14)
	const FString Report = Result.bCrashed
		? FString::Printf(TEXT("CRASH: %s"), *UEnum::GetValueAsString(Result.CrashReason))
		: FString::Printf(TEXT(
			"SCORE %d  (%s)\n"
			"Tracking  LOC %.0f%%  GS %.0f%%   -> %d pts\n"
			"Sink rate %.0f fpm               -> %d pts\n"
			"Zona      %+.0f m del aim point  -> %d pts\n"
			"Eje       %+.1f m                -> %d pts\n"
			"Velocidad %.0f kt                -> %d pts"),
			Result.TotalScore, *Result.Grade,
			Result.LocalizerTrackingPct, Result.GlideslopeTrackingPct, Result.TrackingPoints,
			Result.SinkRateFPM, Result.SinkRatePoints,
			Result.DistanceFromAimPointM, Result.ZonePoints,
			Result.CenterlineOffsetM, Result.CenterlinePoints,
			Result.TouchdownSpeedKt, Result.SpeedPoints);

	UE_LOG(LogTemp, Warning, TEXT("%s"), *Report);
	OnLandingScored.Broadcast(Result);
}