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
	Collision	
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
struct FInputActionValue;
class USkeletalMeshComponent;
class UILSComponent;

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

	//Flight State
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="Flight|State")
	EAircraftState State = EAircraftState::Airborne;
	
	UPROPERTY(BlueprintAssignable, Category="Flight|Events") FOnTouchdown OnTouchdown;
	UPROPERTY(BlueprintAssignable, Category="Flight|Events") FOnCrash OnCrash;
	UPROPERTY(BlueprintAssignable, Category="Flight|Events") FOnStopped OnStopped;


protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	
	// --- Componentes ---
	UPROPERTY(VisibleAnywhere) USpringArmComponent* SpringArm;
	UPROPERTY(VisibleAnywhere) UCameraComponent* Camera;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Aircraft") TObjectPtr<USkeletalMeshComponent> AirframeMesh;
	UPROPERTY(VisibleAnywhere) UILSComponent* ILS;
	UPROPERTY(VisibleAnywhere, Category="Aircraft") USceneComponent* MainGearContact;

	// --- Parámetros de tuning (EditAnywhere para iterar sin recompilar) ---
	UPROPERTY(EditAnywhere, Category="Flight|Forces") float MaxThrustN = 60000.f;
	UPROPERTY(EditAnywhere, Category="Flight|Forces") float LiftCoefficient = 1.5f;
	UPROPERTY(EditAnywhere, Category="Flight|Forces") float DragCoefficient = 0.025f;
	UPROPERTY(EditAnywhere, Category="Flight|Forces") float InducedDragFactor = 0.08f;
	UPROPERTY(EditAnywhere, Category="Flight|Forces") float WingAreaM2 = 50.f;
	UPROPERTY(EditAnywhere, Category="Flight|Forces") float MassKg = 2000.f;

	UPROPERTY(EditAnywhere, Category="Flight|Handling") float PitchRateDeg = 25.f;
	UPROPERTY(EditAnywhere, Category="Flight|Handling") float RollRateDeg = 60.f;
	UPROPERTY(EditAnywhere, Category="Flight|Handling") float YawRateDeg = 10.f;
	UPROPERTY(EditAnywhere, Category="Flight|Handling") float ThrottleResponse = 0.5f;
	UPROPERTY(EditAnywhere, Category="Flight|Handling") float ThrottleRatePerSec = 1.2f;
	UPROPERTY(EditAnywhere, Category="Debug") bool bDrawDebug = false;
	UPROPERTY(EditDefaultsOnly, Category="Input") UInputMappingContext* FlightMappingContext;
	UPROPERTY(EditDefaultsOnly, Category="Input") UInputAction* PitchAction;
	UPROPERTY(EditDefaultsOnly, Category="Input") UInputAction* RollAction;
	UPROPERTY(EditDefaultsOnly, Category="Input") UInputAction* ThrottleAction;

	UPROPERTY(EditAnywhere, Category="Flight|Ground") float BrakeDecelMS2 = 4.f;
	UPROPERTY(EditAnywhere, Category="Flight|Ground") float RollingFrictionMS2 = 0.8f;
	
	// --- Límites de aterrizaje ---
	UPROPERTY(EditAnywhere, Category="Flight|Landing") FName RunwayTag = TEXT("Runway");
	UPROPERTY(EditAnywhere, Category="Flight|Landing") float MaxTouchdownSinkFPM = 1000.f;
	UPROPERTY(EditAnywhere, Category="Flight|Landing") float MaxTouchdownBankDeg = 10.f;
	UPROPERTY(EditAnywhere, Category="Flight|Landing") float MinTouchdownPitchDeg = -2.f;
	
	UPROPERTY(EditDefaultsOnly, Category="UI") TSubclassOf<class UFlightInstrumentWidget> InstrumentsWidgetClass;
	
	bool IsRunway(const FHitResult& Hit) const;
	void HandleTouchdown(const FHitResult& Hit, const FVector& GearOffset);
	void Crash(ECrashReason Reason);
	void CheckStillOnRunway();

public:	
	// Estado
	FVector Velocity = FVector::ZeroVector;   // cm/s en espacio mundo
	float Throttle = 0.6f;
	 float ThrottleTarget = 0.6f; 
	float PitchInput = 0.f, RollInput = 0.f;

	void ApplyFlightForces(float DeltaTime);
	void ApplyRotationInput(float DeltaTime);
	void ApplyGroundRoll(float DeltaTime);

	// Input handlers
	void OnPitch(const FInputActionValue& Value);
	void OnRoll(const FInputActionValue& Value);
	void OnThrottle(const FInputActionValue& Value);
	
	
	// --- Debug / telemetría interna ---
	float DebugAoA = 0.f;
	float DebugLiftN = 0.f;
	float DebugThrustN = 0.f;
	float DebugDragN = 0.f;
	float DebugCL = 0.f;
	float DebugQ = 0.f;
	float DebugSinAoA = 0.f;
	float DebugSpeed = 0.f;

};
