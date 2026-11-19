// Fill out your copyright notice in the Description page of Project Settings.


#include "WindSubsystem.h"

void UWindSubsystem::SetWind(float FromDeg, float SpeedKt, float InGustKt)
{
	BaseFromDeg = FromDeg;
	BaseSpeedKt = FMath::Max(0.f, SpeedKt);
	GustKt      = FMath::Max(0.f, InGustKt);

	// Semilla nueva: cada intento tiene ráfagas distintas
	Seed = FMath::FRandRange(0.f, 1000.f);
	Time = 0.f;
	UpdateCurrent();
}


void UWindSubsystem::Tick(float DeltaTime)
{
	Time += DeltaTime;
	UpdateCurrent();
}

void UWindSubsystem::UpdateCurrent()
{
	// Ruido Perlin: varía suave y sin saltos, como una ráfaga de verdad
	const float Gust01 = 0.5f + 0.5f * FMath::PerlinNoise1D(Seed + Time * 0.35f);   // 0..1, cambia cada ~3 s
	CurrentSpeedKt = BaseSpeedKt + GustKt * Gust01;

	// La dirección también "baila" ±8°, más despacio
	CurrentFromDeg = BaseFromDeg + 8.f * FMath::PerlinNoise1D(Seed + 500.f + Time * 0.15f);

	// Viene de FromDeg → sopla hacia FromDeg + 180
	const FVector BlowDir = FRotator(0.f, CurrentFromDeg + 180.f, 0.f).Vector();
	CurrentVelocity = BlowDir * (CurrentSpeedKt / 1.94384f) * 100.f;   // kt → m/s → cm/s
}