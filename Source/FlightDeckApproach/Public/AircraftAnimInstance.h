#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "AircraftAnimInstance.generated.h"

// Lee el avión cada frame y deja listos los ángulos de superficies, tren y toberas
UCLASS()
class FLIGHTDECKAPPROACH_API UAircraftAnimInstance : public UAnimInstance
{
	GENERATED_BODY()

protected:
	virtual void NativeUpdateAnimation(float DeltaSeconds) override;

	// --- Salidas (el Event Graph del ABP las copia a sus variables) ---
	UPROPERTY(BlueprintReadOnly, Category="Aircraft") float AileronDeg = 0.f;
	UPROPERTY(BlueprintReadOnly, Category="Aircraft") float StabLDeg = 0.f;
	UPROPERTY(BlueprintReadOnly, Category="Aircraft") float StabRDeg = 0.f;
	UPROPERTY(BlueprintReadOnly, Category="Aircraft") float RudderDeg = 0.f;
	UPROPERTY(BlueprintReadOnly, Category="Aircraft") float FlapDeg = 0.f;
	UPROPERTY(BlueprintReadOnly, Category="Aircraft") float GearRetracted = 1.f;   // alpha de las Apply Additive: 1 = arriba
	UPROPERTY(BlueprintReadOnly, Category="Aircraft") float NozzleScale = 1.f;

	// --- Ajustes. Un valor negativo invierte el sentido ---
	UPROPERTY(EditDefaultsOnly, Category="Aircraft|Surfaces") float MaxAileronDeg = 25.f;
	UPROPERTY(EditDefaultsOnly, Category="Aircraft|Surfaces") float MaxStabDeg = 15.f;
	// Los estabilizadores del F-18 también ayudan a alabear (se mueven opuestos)
	UPROPERTY(EditDefaultsOnly, Category="Aircraft|Surfaces") float StabRollMixDeg = 6.f;
	// Un toque de timón al alabear, solo visual
	UPROPERTY(EditDefaultsOnly, Category="Aircraft|Surfaces") float RudderRollMixDeg = 5.f;
	// Los flaps bajan junto con el tren (configuración de aterrizaje)
	UPROPERTY(EditDefaultsOnly, Category="Aircraft|Surfaces") float FlapsDownDeg = 30.f;
	// Qué tan rápido siguen las superficies a la palanca
	UPROPERTY(EditDefaultsOnly, Category="Aircraft|Surfaces") float SurfaceSpeed = 6.f;

	UPROPERTY(EditDefaultsOnly, Category="Aircraft|Nozzles") float NozzleIdleScale = 1.15f;   // abiertas en ralentí
	UPROPERTY(EditDefaultsOnly, Category="Aircraft|Nozzles") float NozzleMaxScale  = 0.85f;   // cerradas a potencia máxima
};