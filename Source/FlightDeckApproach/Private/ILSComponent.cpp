// Fill out your copyright notice in the Description page of Project Settings.


#include "ILSComponent.h"
#include "ILSTransmitter.h"
#include "Kismet/GameplayStatics.h"

UILSComponent::UILSComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}


void UILSComponent::BeginPlay()
{
	Super::BeginPlay();

	if (!Transmitter)
	{
		TArray<AActor*> Found;
		UGameplayStatics::GetAllActorsOfClass(GetWorld(), AILSTransmitter::StaticClass(), Found);
		if (Found.Num() > 0)
		{
			Transmitter = Cast<AILSTransmitter>(Found[0]);
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("ILSComponent: no hay AILSTransmitter en el nivel"));
		}
	}
	
}

void UILSComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	UpdateDeviations();
}

float UILSComponent::GetLocalizerNormalized() const
{
	return FMath::Clamp(LocalizerDeviationDeg / LocalizerFullScaleDeg, -1.f, 1.f);
}

float UILSComponent::GetGlideslopeNormalized() const
{
	return FMath::Clamp(GlideslopeDeviationDeg / GlideslopeFullScaleDeg, -1.f, 1.f);
}

void UILSComponent::UpdateDeviations()
{
	if (!Transmitter)
	{
		bSignalValid = false;
		return;
	}

	const FTransform T = Transmitter->GetActorTransform();

	// Posición del avión en el espacio LOCAL del transmisor.
	// Este cambio de base es el truco que hace toda la matemática trivial:
	//   X = a lo largo del eje de pista (negativa cuando estás en final)
	//   Y = desviación lateral (positiva = a la derecha del eje)
	//   Z = altura sobre el umbral
	const FVector Local = T.InverseTransformPosition(GetOwner()->GetActorLocation());

	const float DistAlong = -Local.X;   // positiva mientras te acercas

	// Sin señal si ya pasaste el umbral o estás detrás del transmisor
	if (DistAlong < 100.f)
	{
		bSignalValid = false;
		return;
	}
	bSignalValid = true;

	// --- LOCALIZER: medido desde su antena, al otro extremo de la pista ---
	const float DistToLoc = DistAlong + Transmitter->LocalizerAntennaMetersFromThreshold * 100.f;
	LocalizerDeviationDeg = FMath::RadiansToDegrees(FMath::Atan2(Local.Y, DistToLoc));

	// --- GLIDESLOPE: medido desde la zona de toque (aim point) ---
	const float DistToAim = DistAlong + Transmitter->AimPointMetersFromThreshold * 100.f;
	const float CurrentAngleDeg = FMath::RadiansToDegrees(FMath::Atan2(Local.Z, DistToAim));
	GlideslopeDeviationDeg = CurrentAngleDeg - Transmitter->GlideslopeAngleDeg;

	// --- DME en millas náuticas ---
	DistanceNM = (DistAlong * 0.01f) / 1852.f;
}

