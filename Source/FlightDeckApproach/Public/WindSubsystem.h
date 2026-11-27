// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "WindSubsystem.generated.h"

/**
 * 
 */
UCLASS()
class FLIGHTDECKAPPROACH_API UWindSubsystem : public UTickableWorldSubsystem
{
	GENERATED_BODY()

public:
	virtual void Tick(float DeltaTime) override;
	virtual TStatId GetStatId() const override
	{
		RETURN_QUICK_DECLARE_CYCLE_STAT(UWindSubsystem, STATGROUP_Tickables);
	}

	// FromDeg: de DÓNDE viene (como en aviación). GustKt: cuánto puede subir sobre la base
	UFUNCTION(BlueprintCallable, Category="Wind") void SetWind(float FromDeg, float SpeedKt, float InGustKt);

	// Velocidad del aire en cm/s, en espacio mundo (hacia DÓNDE sopla)
	FVector GetWindVelocity() const { return CurrentVelocity; }

	UFUNCTION(BlueprintPure, Category="Wind") float GetFromDeg() const { return CurrentFromDeg; }      // con oscilación
	UFUNCTION(BlueprintPure, Category="Wind") float GetSpeedKt() const { return CurrentSpeedKt; }      // con ráfaga actual
	UFUNCTION(BlueprintPure, Category="Wind") float GetBaseFromDeg() const { return BaseFromDeg; }
	UFUNCTION(BlueprintPure, Category="Wind") float GetBaseSpeedKt() const { return BaseSpeedKt; }
	UFUNCTION(BlueprintPure, Category="Wind") float GetGustKt() const { return GustKt; }
	// Lluvia: la decide el GameMode según el preset; la usan el audio y la lluvia visual
	UFUNCTION(BlueprintCallable, Category="Weather") void SetRain(bool bInRain) { bRain = bInRain; }
	UFUNCTION(BlueprintPure, Category="Weather") bool IsRaining() const { return bRain; }

private:
	void UpdateCurrent();

	float BaseFromDeg = 0.f;
	float BaseSpeedKt = 0.f;
	float GustKt = 0.f;

	float CurrentFromDeg = 0.f;
	float CurrentSpeedKt = 0.f;
	FVector CurrentVelocity = FVector::ZeroVector;

	float Time = 0.f;
	float Seed = 0.f;
	bool bRain = false;
};
