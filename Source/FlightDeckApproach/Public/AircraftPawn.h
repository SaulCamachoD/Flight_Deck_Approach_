// Fill out your copyright notice in the Description page of Project Settings.

#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "AircraftPawn.generated.h"

UENUM(BlueprintType)
enum class ECrashReason : uint8
{	
	OffRunway,
	HardLanding,
	ExcessiveBank,
	NoseFirst,
	RunwayExcursion,
	Collision,
	OutOfBounds,
	GearUp
};


UENUM(BlueprintType)
enum class EAircraftState : uint8
{
	Airborne,
	Rolling,
	Stopped,
	Crashed
};

USTRUCT(BlueprintType)
struct FTouchdownData
{
	GENERATED_BODY()
	UPROPERTY(BlueprintReadOnly) float VerticalSpeedFPM = 0.f;
	UPROPERTY(BlueprintReadOnly) float RollDeg = 0.f;
	UPROPERTY(BlueprintReadOnly) float PitchDeg = 0.f;
	UPROPERTY(BlueprintReadOnly) float AirspeedKnots = 0.f;
	UPROPERTY(BlueprintReadOnly) FVector Location = FVector::ZeroVector;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnTouchdown, const FTouchdownData&, Data);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCrash, ECrashReason, Reason);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnStopped);

class UStaticMeshComponent;
class USpringArmComponent;
class UCameraComponent;
class UInputMappingContext;
class UInputAction;
class ULandingScoreComponent;
struct FInputActionValue;
class USkeletalMeshComponent;
class UILSComponent;
class UParticleSystemComponent;
class UPointLightComponent;

UCLASS()
class FLIGHTDECKAPPROACH_API AAircraftPawn : public APawn
{
	GENERATED_BODY()

public:
	AAircraftPawn();
	
	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;

	// --- Estado de vuelo (lectura para UI e ILS) ---
	UFUNCTION(BlueprintPure) float GetAirspeedKnots() const;
	UFUNCTION(BlueprintPure) float GetVerticalSpeedFPM() const;   // pies por minuto
	UFUNCTION(BlueprintPure) float GetPitchDeg() const { return GetActorRotation().Pitch; };
	UFUNCTION(BlueprintPure) float GetRollDeg() const { return GetActorRotation().Roll; };
	UFUNCTION(BlueprintPure) float GetAltitudeFeet() const;
	UFUNCTION(BlueprintPure) float GetThrottle() const { return Throttle; }
	UFUNCTION(BlueprintPure) UILSComponent* GetILS() const { return ILS; }
	UFUNCTION(BlueprintPure) ULandingScoreComponent* GetLandingScore() const { return LandingScore; }
	UFUNCTION(BlueprintPure) float GetHeadingDeg() const { return FRotator::ClampAxis(GetActorRotation().Yaw); }   // 0..360
	UFUNCTION(BlueprintPure) float GetThrottleTarget() const { return ThrottleTarget; }
	UFUNCTION(BlueprintPure) float GetGroundSpeedKnots() const { return Velocity.Size2D() * 0.01f * 1.94384f; }
	
	// --- Tren y mandos (para el AnimBP y el HUD) ---
	UFUNCTION(BlueprintPure) float GetGearExtension() const { return GearExtension; }   // 0 = arriba, 1 = abajo
	UFUNCTION(BlueprintPure) bool IsGearDown() const { return GearExtension >= 1.f; }
	UFUNCTION(BlueprintPure) bool IsGearMoving() const { return GearExtension != (bGearCommandDown ? 1.f : 0.f); }
	UFUNCTION(BlueprintPure) float GetPitchInput() const { return PitchInput; }
	UFUNCTION(BlueprintPure) float GetRollInput() const { return RollInput; }
	
	virtual FVector GetVelocity() const override { return Velocity; }
	void SetInstrumentsVisible(bool bVisible);
	
	
	//Flight State
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="Flight|State")
	EAircraftState State = EAircraftState::Airborne;
	
	UPROPERTY(BlueprintAssignable, Category="Flight|Events") FOnTouchdown OnTouchdown;
	UPROPERTY(BlueprintAssignable, Category="Flight|Events") FOnCrash OnCrash;
	UPROPERTY(BlueprintAssignable, Category="Flight|Events") FOnStopped OnStopped;


protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	
	virtual void NotifyControllerChanged() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	
	// --- Componentes ---
	UPROPERTY(VisibleAnywhere) USpringArmComponent* SpringArm;
	UPROPERTY(VisibleAnywhere) UCameraComponent* Camera;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Aircraft") TObjectPtr<USkeletalMeshComponent> AirframeMesh;
	
	// Llamas de las toberas (FX_Engine_01_F18C), pegadas a los huesos de las toberas
	UPROPERTY(VisibleAnywhere, Category="Aircraft|FX") UParticleSystemComponent* EngineFlameL;
	UPROPERTY(VisibleAnywhere, Category="Aircraft|FX") UParticleSystemComponent* EngineFlameR;
	
	// Luz de cada tobera: se ve desde atrás, donde la llama queda de canto
	UPROPERTY(VisibleAnywhere, Category="Aircraft|FX") UPointLightComponent* EngineLightL;
	UPROPERTY(VisibleAnywhere, Category="Aircraft|FX") UPointLightComponent* EngineLightR;
	
	// --- Choque ---
	// Malla destruida: oculta hasta el choque
	UPROPERTY(VisibleAnywhere, Category="Aircraft|Crash") UStaticMeshComponent* WreckMesh;
	// Fuego y humo sobre los restos
	UPROPERTY(VisibleAnywhere, Category="Aircraft|Crash") UParticleSystemComponent* WreckFire;
	// Cuánto del ángulo del impacto conservan los restos (0 = plano, 1 = igual que chocó)
	UPROPERTY(EditDefaultsOnly, Category="Aircraft|Crash", meta=(ClampMin="0", ClampMax="1")) float WreckKeepAngle = 0.3f;
	// Inclinación máxima de los restos, para que no se entierren
	UPROPERTY(EditDefaultsOnly, Category="Aircraft|Crash") float WreckMaxTiltDeg = 15.f;
	// true = bajar los restos hasta el suelo (ideal si quedan planos)
	// false = dejarlos justo donde chocó (ideal con el ángulo completo)
	UPROPERTY(EditDefaultsOnly, Category="Aircraft|Crash") bool bWreckSnapToGround = false;
	
	// Intensidad con throttle al 100 % (candelas)
	UPROPERTY(EditDefaultsOnly, Category="Aircraft|FX") float EngineLightMaxCandelas = 1500.f;

	// Por debajo de este throttle la llama se apaga
	UPROPERTY(EditDefaultsOnly, Category="Aircraft|FX") float FlameMinThrottle = 0.05f;
	// Escala con el throttle mínimo y con el máximo. X = largo; Y, Z = grosor
	UPROPERTY(EditDefaultsOnly, Category="Aircraft|FX") FVector FlameIdleScale = FVector(0.3f, 0.7f, 0.7f);
	UPROPERTY(EditDefaultsOnly, Category="Aircraft|FX") FVector FlameMaxScale  = FVector(1.f, 1.f, 1.f);
	// Cuánto "respira" la llama (0 = fija)
	UPROPERTY(EditDefaultsOnly, Category="Aircraft|FX") float FlameFlicker = 0.06f;
	UPROPERTY(VisibleAnywhere) UILSComponent* ILS;
	UPROPERTY(VisibleAnywhere) ULandingScoreComponent* LandingScore;
	UPROPERTY(VisibleAnywhere, Category="Aircraft") USceneComponent* MainGearContact;

	// --- Parámetros de tuning (EditAnywhere para iterar sin recompilar) ---
	UPROPERTY(EditAnywhere, Category="Flight|Forces") float MaxThrustN = 12000.f;
	UPROPERTY(EditAnywhere, Category="Flight|Forces") float LiftCoefficient = 1.5f;
	UPROPERTY(EditAnywhere, Category="Flight|Forces") float DragCoefficient = 0.05f;
	UPROPERTY(EditAnywhere, Category="Flight|Forces") float InducedDragFactor = 0.08f;
	UPROPERTY(EditAnywhere, Category="Flight|Forces") float WingAreaM2 = 50.f;
	UPROPERTY(EditAnywhere, Category="Flight|Forces") float MassKg = 2000.f;
	
	// Fuerza lateral del fuselaje: alinea poco a poco la trayectoria con el morro tras una ráfaga
	UPROPERTY(EditAnywhere, Category="Flight|Aero") float SideForceCoefficient = 0.5f;
	

	UPROPERTY(EditAnywhere, Category="Flight|Handling") float PitchRateDeg = 25.f;
	UPROPERTY(EditAnywhere, Category="Flight|Handling") float RollRateDeg = 60.f;
	UPROPERTY(EditAnywhere, Category="Flight|Handling") float YawRateDeg = 10.f;
	UPROPERTY(EditAnywhere, Category="Flight|Handling") float ThrottleResponse = 0.5f;
	UPROPERTY(EditAnywhere, Category="Flight|Handling") float ThrottleRatePerSec = 1.2f;
	UPROPERTY(EditAnywhere, Category="Flight|Start") float StartSpeedKnots = 140.f;
	UPROPERTY(EditAnywhere, Category="Flight|Start") float StartFlightPathDeg = -3.f;
	UPROPERTY(EditAnywhere, Category="Flight|Start") float StartPitchDeg = -2.2f;
	
	UPROPERTY(EditAnywhere, Category="Debug") bool bDrawDebug = false;
	UPROPERTY(EditAnywhere, Category="Debug") bool bShowDebugText = false;
	UPROPERTY(EditDefaultsOnly, Category="Input") UInputMappingContext* FlightMappingContext;
	UPROPERTY(EditDefaultsOnly, Category="Input") UInputAction* PitchAction;
	UPROPERTY(EditDefaultsOnly, Category="Input") UInputAction* RollAction;
	UPROPERTY(EditDefaultsOnly, Category="Input") UInputAction* ThrottleAction;
	
	UPROPERTY(EditDefaultsOnly, Category="Input") UInputAction* GearAction;
	
	UPROPERTY(EditDefaultsOnly, Category="Flight|Gear") float GearTransitSeconds = 4.f;
	UPROPERTY(EditDefaultsOnly, Category="Flight|Gear") bool bStartGearDown = false;
	UPROPERTY(EditDefaultsOnly, Category="Input") UInputAction* PauseAction;

	UPROPERTY(EditAnywhere, Category="Flight|Ground") float BrakeDecelMS2 = 4.f;
	UPROPERTY(EditAnywhere, Category="Flight|Ground") float RollingFrictionMS2 = 0.8f;
	UPROPERTY(EditAnywhere, Category="Flight|Ground") float GroundAlignRateDeg = 10.f;
	
	// --- Límites de aterrizaje ---
	UPROPERTY(EditAnywhere, Category="Flight|Landing") FName RunwayTag = TEXT("Runway");
	UPROPERTY(EditAnywhere, Category="Flight|Landing") float MaxTouchdownSinkFPM = 1000.f;
	UPROPERTY(EditAnywhere, Category="Flight|Landing") float MaxTouchdownBankDeg = 10.f;
	UPROPERTY(EditAnywhere, Category="Flight|Landing") float MinTouchdownPitchDeg = -2.f;
	
	// Límites de la aproximación: fuera de aquí el intento termina
	UPROPERTY(EditAnywhere, Category="Flight|Bounds") float RunwayLengthM = 3000.f;   // pasarla en el aire = fin
	UPROPERTY(EditAnywhere, Category="Flight|Bounds") float MaxDistanceNM = 7.f;      // radio desde el umbral
	UPROPERTY(EditAnywhere, Category="Flight|Bounds") float MinWorldZ = -10000.f;     // cayendo fuera del mapa
	
	UPROPERTY(EditDefaultsOnly, Category="UI") TSubclassOf<class UFlightInstrumentWidget> InstrumentsWidgetClass;
	
	bool IsRunway(const FHitResult& Hit) const;
	void HandleTouchdown(const FHitResult& Hit, const FVector& GearOffset);
	void Crash(ECrashReason Reason);
	void CheckStillOnRunway();
	void CheckOutOfBounds();

private:	
	// Estado
	FVector Velocity = FVector::ZeroVector;   // cm/s en espacio mundo
	float Throttle = 0.6f;
	 float ThrottleTarget = 0.6f; 
	float PitchInput = 0.f, RollInput = 0.f;
	
	bool bGearCommandDown = false;   
	float GearExtension = 0.f;       
	void UpdateGear(float DeltaTime);
	void UpdateEngineFlames();
	void ShowWreck();
	
	void ApplyFlightForces(float DeltaTime);
	void ApplyRotationInput(float DeltaTime);
	void ApplyGroundRoll(float DeltaTime);

	// Input handlers
	void OnPitch(const FInputActionValue& Value);
	void OnRoll(const FInputActionValue& Value);
	void OnThrottle(const FInputActionValue& Value);
	void OnPause();
	void OnGear();
	
	// --- Debug / telemetría interna ---
	float DebugAoA = 0.f;
	float DebugLiftN = 0.f;
	float DebugThrustN = 0.f;
	float DebugDragN = 0.f;
	float DebugCL = 0.f;
	float DebugQ = 0.f;
	float DebugSinAoA = 0.f;
	float DebugSpeed = 0.f;
	
	void SetupForController();

	UPROPERTY() TObjectPtr<class UFlightInstrumentWidget> InstrumentsWidget;
};
