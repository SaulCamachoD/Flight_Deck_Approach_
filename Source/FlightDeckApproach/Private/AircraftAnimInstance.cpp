#include "AircraftAnimInstance.h"
#include "AircraftPawn.h"

void UAircraftAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);

	// En el editor de animación no hay pawn: se queda la pose de reposo
	const AAircraftPawn* Aircraft = Cast<AAircraftPawn>(TryGetPawnOwner());
	if (!Aircraft) return;

	const float Pitch = Aircraft->GetPitchInput();
	const float Roll  = Aircraft->GetRollInput();

	// Las superficies no saltan: persiguen su objetivo suavemente
	auto Ease = [&](float& Current, float Target)
	{
		Current = FMath::FInterpTo(Current, Target, DeltaSeconds, SurfaceSpeed);
	};
	Ease(AileronDeg, Roll * MaxAileronDeg);
	Ease(StabLDeg,   Pitch * MaxStabDeg + Roll * StabRollMixDeg);
	Ease(StabRDeg,   Pitch * MaxStabDeg - Roll * StabRollMixDeg);
	Ease(RudderDeg,  Roll * RudderRollMixDeg);

	const float Gear = Aircraft->GetGearExtension();
	FlapDeg       = FlapsDownDeg * Gear;
	GearRetracted = 1.f - Gear;
	NozzleScale   = FMath::Lerp(NozzleIdleScale, NozzleMaxScale, Aircraft->GetThrottle());
}