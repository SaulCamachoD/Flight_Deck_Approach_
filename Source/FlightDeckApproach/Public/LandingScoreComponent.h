// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "AircraftPawn.h" 
#include "LandingScoreComponent.generated.h"

USTRUCT(BlueprintType)
struct FLandingResult
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly) bool bCrashed = false;
	UPROPERTY(BlueprintReadOnly) ECrashReason CrashReason = ECrashReason::Collision;
	UPROPERTY(BlueprintReadOnly) int32 TotalScore = 0;      // 0..1000
	UPROPERTY(BlueprintReadOnly) FString Grade;

	// --- Aproximación ---
	UPROPERTY(BlueprintReadOnly) float LocalizerTrackingPct = 0.f;
	UPROPERTY(BlueprintReadOnly) float GlideslopeTrackingPct = 0.f;

	// --- Toque ---
	UPROPERTY(BlueprintReadOnly) float SinkRateFPM = 0.f;
	UPROPERTY(BlueprintReadOnly) float DistanceFromAimPointM = 0.f;   // + = largo, - = corto
	UPROPERTY(BlueprintReadOnly) float CenterlineOffsetM = 0.f;       // + = a la derecha del eje
	UPROPERTY(BlueprintReadOnly) float TouchdownSpeedKt = 0.f;

	// --- Puntos por apartado ---
	UPROPERTY(BlueprintReadOnly) int32 TrackingPoints = 0;
	UPROPERTY(BlueprintReadOnly) int32 SinkRatePoints = 0;
	UPROPERTY(BlueprintReadOnly) int32 ZonePoints = 0;
	UPROPERTY(BlueprintReadOnly) int32 CenterlinePoints = 0;
	UPROPERTY(BlueprintReadOnly) int32 SpeedPoints = 0;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnLandingScored, const FLandingResult&, Result);

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class FLIGHTDECKAPPROACH_API ULandingScoreComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	ULandingScoreComponent();
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	// Se emite una sola vez: al detenerse el avión o al estrellarse
	UPROPERTY(BlueprintAssignable, Category="Score") FOnLandingScored OnLandingScored;

	// Tracking acumulado hasta ahora, para el panel APPROACH SCORE en vivo (Parte 15)
	UFUNCTION(BlueprintPure) float GetLiveTrackingPct() const;
	UFUNCTION(BlueprintPure) bool HasResult() const { return bHasResult; }
	UFUNCTION(BlueprintPure) const FLandingResult& GetResult() const { return Result; }

protected:
	// Called when the game starts
	virtual void BeginPlay() override;
	
	// --- Reparto de puntos (suman 1000) ---
	UPROPERTY(EditAnywhere, Category="Score|Points") int32 MaxTrackingPoints   = 300;
	UPROPERTY(EditAnywhere, Category="Score|Points") int32 MaxSinkRatePoints   = 250;
	UPROPERTY(EditAnywhere, Category="Score|Points") int32 MaxZonePoints       = 200;
	UPROPERTY(EditAnywhere, Category="Score|Points") int32 MaxCenterlinePoints = 150;
	UPROPERTY(EditAnywhere, Category="Score|Points") int32 MaxSpeedPoints      = 100;

	// --- Tolerancias: hasta "Full" puntuación completa, en "Zero" ya no hay puntos ---
	UPROPERTY(EditAnywhere, Category="Score|Tolerances") float SinkFullFPM = 200.f;
	UPROPERTY(EditAnywhere, Category="Score|Tolerances") float SinkZeroFPM = 1000.f;
	UPROPERTY(EditAnywhere, Category="Score|Tolerances") float ZoneFullM = 50.f;
	UPROPERTY(EditAnywhere, Category="Score|Tolerances") float ZoneZeroM = 400.f;
	UPROPERTY(EditAnywhere, Category="Score|Tolerances") float CenterFullM = 2.f;
	UPROPERTY(EditAnywhere, Category="Score|Tolerances") float CenterZeroM = 20.f;
	UPROPERTY(EditAnywhere, Category="Score|Tolerances") float TargetTouchdownKt = 130.f;
	UPROPERTY(EditAnywhere, Category="Score|Tolerances") float SpeedFullKt = 10.f;
	UPROPERTY(EditAnywhere, Category="Score|Tolerances") float SpeedZeroKt = 30.f;

	// Cerca del umbral el glide slope se vuelve hipersensible: dejamos de medir antes
	UPROPERTY(EditAnywhere, Category="Score|Tracking") float MinSampleDistanceNM = 0.3f;

private:
	UFUNCTION() void HandleTouchdown(const FTouchdownData& Data);
	UFUNCTION() void HandleCrash(ECrashReason Reason);
	UFUNCTION() void HandleStopped();

	void Finish();
	static float Score01(float Value, float FullAt, float ZeroAt);
	static FString GradeFor(int32 Score);

	UPROPERTY() TObjectPtr<AAircraftPawn> Pawn;
	FLandingResult Result;
	bool bHasResult = false;

	// Sumas ponderadas por tiempo de |desviación normalizada|
	float SumLocDev = 0.f;
	float SumGsDev = 0.f;
	float SampleTime = 0.f;
		
};
