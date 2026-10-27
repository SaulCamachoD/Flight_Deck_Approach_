// Fill out your copyright notice in the Description page of Project Settings.


#include "AAircraftPaw.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Components/SkeletalMeshComponent.h"
#include "InputActionValue.h"
#include "GameFramework/PlayerController.h"
#include "UILSComponent.h"
#include "Engine/Engine.h"
#include "DrawDebugHelpers.h"

// Sets default values
AAAircraftPaw::AAAircraftPaw()
{
 	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
    
    USceneComponent* Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
    SetRootComponent(Root);

    AirframeMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("AirframeMesh"));
    AirframeMesh->SetupAttachment(Root);
    AirframeMesh->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    AirframeMesh->SetCollisionResponseToAllChannels(ECR_Block);
    
    SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
    SpringArm->SetupAttachment(Root);        // ← al Root, no al mesh
    SpringArm->TargetArmLength = 1200.f;
    SpringArm->SocketOffset = FVector(0.f, 0.f, 300.f);
    SpringArm->bEnableCameraLag = true;
    SpringArm->CameraLagSpeed = 8.f;
    SpringArm->bInheritRoll = false;
    SpringArm->bDoCollisionTest = false;

    Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
    Camera->SetupAttachment(SpringArm);
    
    ILS = CreateDefaultSubobject<UUILSComponent>(TEXT("ILS"));
    MainGearContact = CreateDefaultSubobject<USceneComponent>(TEXT("MainGearContact"));
    MainGearContact->SetupAttachment(Root);
    AutoPossessPlayer = EAutoReceiveInput::Player0; 

}

// Called when the game starts or when spawned
void AAAircraftPaw::BeginPlay()
{
	Super::BeginPlay();
    
    Velocity = GetActorForwardVector() * 6700.f;
    ThrottleTarget = Throttle;
    
    if (APlayerController* PC = Cast<APlayerController>(GetController()))
    {
        if (UEnhancedInputLocalPlayerSubsystem* Subsystem =
            ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PC->GetLocalPlayer()))
        {
            if (FlightMappingContext)
            {
                Subsystem->AddMappingContext(FlightMappingContext, 0);
            }
        }
    }
	
}

bool AAAircraftPaw::IsRunway(const FHitResult& Hit) const
{
    const AActor* HitActor = Hit.GetActor();
    const UPrimitiveComponent* HitComp = Hit.GetComponent();
    return (HitActor && HitActor->ActorHasTag(RunwayTag))
        || (HitComp && HitComp->ComponentHasTag(RunwayTag));
}

void AAAircraftPaw::HandleTouchdown(const FHitResult& Hit, const FVector& GearOffset)
{
    SetActorLocation(Hit.Location - GearOffset);

    const float VS    = GetVerticalSpeedFPM();
    const float Roll  = GetRollDeg();
    const float Pitch = GetPitchDeg();

    UE_LOG(LogTemp, Warning, TEXT("TOUCHDOWN — VS: %.0f fpm | Roll: %.1f | Pitch: %.1f | IAS: %.0f kt"),
        VS, Roll, Pitch, GetAirspeedKnots());

    if (!IsRunway(Hit))                          { Crash(ECrashReason::OffRunway);     return; }
    if (-VS > MaxTouchdownSinkFPM)               { Crash(ECrashReason::HardLanding);   return; }
    if (FMath::Abs(Roll) > MaxTouchdownBankDeg)  { Crash(ECrashReason::ExcessiveBank); return; }
    if (Pitch < MinTouchdownPitchDeg)            { Crash(ECrashReason::NoseFirst);     return; }

    // Aterrizaje válido
    Velocity.Z = 0.f;
    State = EAircraftState::Rolling;
}

void AAAircraftPaw::Crash(ECrashReason Reason)
{
    if (State == EAircraftState::Crashed) return;

    State = EAircraftState::Crashed;
    Velocity = FVector::ZeroVector;
    Throttle = ThrottleTarget = 0.f;

    UE_LOG(LogTemp, Error, TEXT("CRASH — %s"), *UEnum::GetValueAsString(Reason));

}

void AAAircraftPaw::CheckStillOnRunway()
{
    // Trace corto hacia abajo desde el tren: si ya no hay pista debajo, se salió
    const FVector Start = MainGearContact->GetComponentLocation() + FVector(0.f, 0.f, 50.f);
    const FVector End   = Start - FVector(0.f, 0.f, 300.f);

    FCollisionQueryParams Params;
    Params.AddIgnoredActor(this);

    FHitResult Hit;
    const bool bHit = GetWorld()->LineTraceSingleByChannel(Hit, Start, End, ECC_WorldStatic, Params);

    if (!bHit || !IsRunway(Hit))
    {
        Crash(ECrashReason::RunwayExcursion);
    }
}

void AAAircraftPaw::ApplyFlightForces(float DeltaTime)
{
	const FVector Fwd = GetActorForwardVector();
    const FVector Up  = GetActorUpVector();

    // Velocidad en m/s para la aerodinámica (UE trabaja en cm)
    const FVector VelMS = Velocity * 0.01f;
    const float Speed = VelMS.Size();
    DebugSpeed = Speed; 
    // --- Ángulo de ataque (AoA): ángulo entre el eje longitudinal y el vector velocidad,
    // proyectado en el plano vertical del avión ---
    float AoADeg = 0.f;
    if (Speed > 1.f)
    {
        const FVector VelDir = VelMS.GetSafeNormal();
        const float SinAoA = FVector::DotProduct(FVector::CrossProduct(VelDir, Fwd),GetActorRightVector());
        DebugSinAoA = SinAoA;
        AoADeg = -FMath::RadiansToDegrees(FMath::Asin(FMath::Clamp(SinAoA, -1.f, 1.f)));
    }
    DebugAoA = AoADeg;
    // --- Presión dinámica: q = 0.5 * rho * V^2 ---
    const float Rho = 1.225f; // densidad del aire a nivel del mar, kg/m^3
    const float Q = 0.5f * Rho * Speed * Speed;
    DebugQ = Q; 
    // --- Sustentación: CL crece linealmente con AoA hasta la pérdida (~15°) ---
    const float StallAoA = 15.f;
    float CL = LiftCoefficient * (AoADeg / 10.f);
    if (FMath::Abs(AoADeg) > StallAoA)
    {
        // Pérdida: la sustentación cae bruscamente. Simple pero se SIENTE.
        CL *= FMath::Max(0.f, 1.f - (FMath::Abs(AoADeg) - StallAoA) * 0.15f);
    }
    DebugCL = CL; 
    const float LiftN = Q * WingAreaM2 * CL;
    DebugLiftN = LiftN;

    // --- Arrastre: parásito + inducido (el inducido crece con CL^2) ---
    const float CD = DragCoefficient + InducedDragFactor * CL * CL;
    const float DragN = Q * WingAreaM2 * CD;
    DebugDragN = DragN;
    // --- Empuje ---
    const float ThrustN = Throttle * MaxThrustN;
    DebugThrustN = ThrustN;
    // --- Suma de fuerzas (Newtons) → aceleración (m/s^2) ---
    FVector ForceN = FVector::ZeroVector;
    ForceN += Fwd * ThrustN;
    ForceN += Up * LiftN;
    if (Speed > 0.1f)
    {
        ForceN -= VelMS.GetSafeNormal() * DragN;
    }
    ForceN += FVector(0, 0, -9.81f * MassKg); // gravedad

    // Viento del subsistema (ver Fase 3)
    //if (UWindSubsystem* Wind = GetWorld()->GetSubsystem<UWindSubsystem>())
    //{
      //  ForceN += Wind->GetWindForceOn(MassKg, VelMS);
    //}

    const FVector AccelMS2 = ForceN / MassKg;

    // Integración semi-implícita: primero velocidad, luego posición
    Velocity += AccelMS2 * DeltaTime * 100.f; // de vuelta a cm/s

    const FVector Start = GetActorLocation();
    const FVector Delta = Velocity * DeltaTime;
    const FVector End = Start + Delta;

    FHitResult GroundHit;
    FCollisionQueryParams Params;
    Params.AddIgnoredActor(this);
    
    const FVector GearStart = MainGearContact->GetComponentLocation();
    const FVector GearEnd = GearStart + Delta;

    if (bDrawDebug)
    {
        DrawDebugLine(GetWorld(), GearStart, GearEnd, FColor::Red, false, 0.f, 0, 2.f);
    }

    if (GetWorld()->LineTraceSingleByChannel(GroundHit, GearStart, GearEnd, ECC_WorldStatic, Params))
    {
        HandleTouchdown(GroundHit, GearStart - Start);
        return;
    }
    
    FHitResult BodyHit;
    if (GetWorld()->LineTraceSingleByChannel(BodyHit, Start, End, ECC_WorldStatic, Params))
    {
        SetActorLocation(BodyHit.Location);
        Crash(ECrashReason::Collision);
        return;
    }

    AddActorWorldOffset(Delta, false);
}

void AAAircraftPaw::ApplyRotationInput(float DeltaTime)
{
    FRotator Delta(PitchInput * PitchRateDeg * DeltaTime,
                   0.f,
                   RollInput * RollRateDeg * DeltaTime);
    AddActorLocalRotation(Delta);

    // Yaw coordinado: el avión "sigue" suavemente al banking (giro coordinado de mentira)
    const float RollDeg = GetActorRotation().Roll;
    const float YawRate = FMath::Sin(FMath::DegreesToRadians(RollDeg)) * YawRateDeg;
    AddActorWorldRotation(FRotator(0.f, YawRate * DeltaTime, 0.f));
}

void AAAircraftPaw::ApplyGroundRoll(float DeltaTime)
{
    // Nivelar la actitud: el avión se asienta sobre el tren
    FRotator R = GetActorRotation();
    R.Pitch = FMath::FInterpTo(R.Pitch, 0.f, DeltaTime, 3.f);
    R.Roll  = FMath::FInterpTo(R.Roll,  0.f, DeltaTime, 3.f);
    SetActorRotation(R);

    // Fricción de rodadura + frenos (usamos el throttle bajo como freno)
    const float DecelMS2 = RollingFrictionMS2 + BrakeDecelMS2 * (1.f - Throttle);
    const float SpeedCMS = Velocity.Size();
    const float NewSpeed = FMath::Max(0.f, SpeedCMS - DecelMS2 * 100.f * DeltaTime);

    Velocity = (SpeedCMS > 1.f) ? Velocity.GetSafeNormal() * NewSpeed : FVector::ZeroVector;

    // Rodar en línea recta siguiendo el morro
    if (NewSpeed > 1.f)
    {
        FVector Dir = GetActorForwardVector();
        Dir.Z = 0.f;
        Velocity = Dir.GetSafeNormal() * NewSpeed;
        AddActorWorldOffset(Velocity * DeltaTime, false);
        CheckStillOnRunway();
    }
    else
    {
        Velocity = FVector::ZeroVector;
        State = EAircraftState::Stopped;
        UE_LOG(LogTemp, Warning, TEXT("STOPPED — aterrizaje completado"));
    }
}

void AAAircraftPaw::OnPitch(const FInputActionValue& Value)
{
    PitchInput = Value.Get<float>();
}

void AAAircraftPaw::OnRoll(const FInputActionValue& Value)
{
    RollInput = Value.Get<float>();
}

void AAAircraftPaw::OnThrottle(const FInputActionValue& Value)
{
    const float Dt = GetWorld()->GetDeltaSeconds();
    ThrottleTarget = FMath::Clamp(ThrottleTarget + Value.Get<float>() * ThrottleRatePerSec * Dt, 0.f, 1.f);
}

// Called every frame
void AAAircraftPaw::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
    
    Throttle = FMath::FInterpConstantTo(Throttle, ThrottleTarget, DeltaTime, ThrottleResponse);
    switch (State)
    {
    case EAircraftState::Airborne:
        ApplyRotationInput(DeltaTime);
        ApplyFlightForces(DeltaTime);
        break;
    case EAircraftState::Rolling:
        ApplyGroundRoll(DeltaTime);
        break;
    default: // Stopped / Crashed: quieto
        break;
    }
    
    if (GEngine)
    {
        const float WeightN = MassKg * 9.81f;
        const float LiftRatio = (WeightN > 0.f) ? DebugLiftN / WeightN : 0.f;

        GEngine->AddOnScreenDebugMessage(1, 0.f, FColor::Cyan,
            FString::Printf(TEXT(
                "IAS: %.0f kt | ALT: %.0f ft | VS: %.0f fpm | THR: %.2f\n"
                "AoA: %.2f deg | Pitch: %.1f | Roll: %.1f\n"
                "LIFT: %.0f N  vs  PESO: %.0f N   (ratio %.2f)\n"
                "Thrust: %.0f N | Drag: %.0f N\n"
                "Q: %.0f | CL: %.4f | SinAoA-raw: %.4f | Speed(m/s): %.1f\n"    
                "PitchInput: %.2f | RollInput: %.2f\n"
                "State: %s"),
                GetAirspeedKnots(), GetAltitudeFeet(), GetVerticalSpeedFPM(), Throttle,
                DebugAoA, GetPitchDeg(), GetRollDeg(),
                DebugLiftN, WeightN, LiftRatio,
                DebugThrustN, DebugDragN,
                DebugQ, DebugCL, DebugSinAoA, DebugSpeed,                          
                PitchInput, RollInput,
                *UEnum::GetValueAsString(State)));
    }
    
    if (GEngine && ILS && ILS->IsSignalValid())
    {
        const float Loc = ILS->GetLocalizerDeviationDeg();
        const float GS  = ILS->GetGlideslopeDeviationDeg();

        GEngine->AddOnScreenDebugMessage(2, 0.f, FColor::Yellow,
            FString::Printf(TEXT(
                "ILS  DME: %.2f NM\n"
                "LOC: %+.2f deg (%s)  [aguja %+.2f]\n"
                "GS:  %+.2f deg (%s)  [aguja %+.2f]"),
                ILS->GetDistanceNM(),
                Loc, Loc > 0 ? TEXT("estas a la IZQ, corrige der") : TEXT("estas a la DER, corrige izq"),
                ILS->GetLocalizerNormalized(),
                GS, GS > 0 ? TEXT("ALTO, baja") : TEXT("BAJO, sube"),
                ILS->GetGlideslopeNormalized()));
    }

}

// Called to bind functionality to input
void AAAircraftPaw::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
    
    if (UEnhancedInputComponent* EIC = Cast<UEnhancedInputComponent>(PlayerInputComponent))
    {
        EIC->BindAction(PitchAction, ETriggerEvent::Triggered, this, &AAAircraftPaw::OnPitch);
        EIC->BindAction(PitchAction, ETriggerEvent::Completed, this, &AAAircraftPaw::OnPitch);

        EIC->BindAction(RollAction, ETriggerEvent::Triggered, this, &AAAircraftPaw::OnRoll);
        EIC->BindAction(RollAction, ETriggerEvent::Completed, this, &AAAircraftPaw::OnRoll);

        EIC->BindAction(ThrottleAction, ETriggerEvent::Triggered, this, &AAAircraftPaw::OnThrottle);
    }

}

float AAAircraftPaw::GetAirspeedKnots() const
{
    return Velocity.Size() * 0.01f * 1.94384f;
}

float AAAircraftPaw::GetVerticalSpeedFPM() const
{
    return Velocity.Z * 0.01f * 196.85f;
}

float AAAircraftPaw::GetAltitudeFeet() const
{
    return GetActorLocation().Z * 0.01f * 3.28084f;
}

