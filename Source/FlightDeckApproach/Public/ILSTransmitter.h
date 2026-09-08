// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ILSTransmitter.generated.h"

class UArrowComponent;

UCLASS()
class FLIGHTDECKAPPROACH_API AILSTransmitter : public AActor
{
	GENERATED_BODY()
	
public:	
	AILSTransmitter();

protected:
	
	UPROPERTY(VisibleAnywhere) UArrowComponent* DirectionArrow;

public:	
	
	// Ángulo de la senda de planeo. 3° es el estándar en aviación civil.
	UPROPERTY(EditAnywhere, Category="ILS") float GlideslopeAngleDeg = 3.0f;

	// Rumbo de pista, solo informativo para el HUD (ej. 090 para rumbo este)
	UPROPERTY(EditAnywhere, Category="ILS") float RunwayHeadingDeg = 0.f;

	// Zona de toque ideal, medida desde el umbral (300 m es el estándar)
	UPROPERTY(EditAnywhere, Category="ILS") float AimPointMetersFromThreshold = 300.f;

};
