// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "UILSComponent.generated.h"

class AILSTransmitter;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class FLIGHTDECKAPPROACH_API UUILSComponent : public UActorComponent
{
	GENERATED_BODY()

public:	

	UUILSComponent();
	
	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	// --- Lectura para HUD y debug ---
	UFUNCTION(BlueprintPure) bool  IsSignalValid() const { return bSignalValid; }
	UFUNCTION(BlueprintPure) float GetLocalizerDeviationDeg() const { return LocalizerDeviationDeg; }
	UFUNCTION(BlueprintPure) float GetGlideslopeDeviationDeg() const { return GlideslopeDeviationDeg; }
	UFUNCTION(BlueprintPure) float GetDistanceNM() const { return DistanceNM; }

	// Desviaciones normalizadas a [-1, 1] usando las escalas full-scale reales.
	// Esto es lo que mueven las agujas del CDI.
	UFUNCTION(BlueprintPure) float GetLocalizerNormalized() const;
	UFUNCTION(BlueprintPure) float GetGlideslopeNormalized() const;
	
protected:
	// Si se deja vacío, se busca automáticamente el primero en el nivel.
	UPROPERTY(EditAnywhere, Category="ILS") AILSTransmitter* Transmitter = nullptr;

	// Escalas full-scale reales del ILS (CAT I).
	UPROPERTY(EditAnywhere, Category="ILS") float LocalizerFullScaleDeg = 2.5f;
	UPROPERTY(EditAnywhere, Category="ILS") float GlideslopeFullScaleDeg = 0.7f;

private:
	void UpdateDeviations();

	bool  bSignalValid = false;
	float LocalizerDeviationDeg = 0.f;
	float GlideslopeDeviationDeg = 0.f;
	float DistanceNM = 0.f;
};
