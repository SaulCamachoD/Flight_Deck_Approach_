// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "AAircraftPaw.generated.h"

class UStaticMeshComponent;
class USpringArmComponent;
class UCameraComponent;
class UInputMappingContext;
class UInputAction;
struct FInputActionValue;
class USkeletalMeshComponent;
class UUILSComponent;

UCLASS()
class FLIGHTDECKAPPROACH_API AAAircraftPaw : public APawn
{
	GENERATED_BODY()

public:
	AAAircraftPaw();
	
	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;

	// --- Estado de vuelo (lectura para UI e ILS) ---
	UFUNCTION(BlueprintPure) float GetAirspeedKnots() const;
	UFUNCTION(BlueprintPure) float GetVerticalSpeedFPM() const;   // pies por minuto
	UFUNCTION(BlueprintPure) float GetPitchDeg() const { return GetActorRotation().Pitch; };
	UFUNCTION(BlueprintPure) float GetRollDeg() const { return GetActorRotation().Roll; };
	UFUNCTION(BlueprintPure) float GetAltitudeFeet() const;
	UFUNCTION(BlueprintPure) float GetThrottle() const { return Throttle; }


protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	
	// --- Componentes ---
	UPROPERTY(VisibleAnywhere) USpringArmComponent* SpringArm;
	UPROPERTY(VisibleAnywhere) UCameraComponent* Camera;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Aircraft") TObjectPtr<USkeletalMeshComponent> AirframeMesh;
	UPROPERTY(VisibleAnywhere) UUILSComponent* ILS;

	// --- Parámetros de tuning (EditAnywhere para iterar sin recompilar) ---
	UPROPERTY(EditAnywhere, Category="Flight|Forces") float MaxThrustN = 60000.f;
	UPROPERTY(EditAnywhere, Category="Flight|Forces") float LiftCoefficient = 1.2f;
	UPROPERTY(EditAnywhere, Category="Flight|Forces") float DragCoefficient = 0.025f;
	UPROPERTY(EditAnywhere, Category="Flight|Forces") float InducedDragFactor = 0.08f;
	UPROPERTY(EditAnywhere, Category="Flight|Forces") float WingAreaM2 = 25.f;
	UPROPERTY(EditAnywhere, Category="Flight|Forces") float MassKg = 5000.f;

	UPROPERTY(EditAnywhere, Category="Flight|Handling") float PitchRateDeg = 25.f;
	UPROPERTY(EditAnywhere, Category="Flight|Handling") float RollRateDeg = 60.f;
	UPROPERTY(EditAnywhere, Category="Flight|Handling") float YawRateDeg = 10.f;
	UPROPERTY(EditAnywhere, Category="Flight|Handling") float ThrottleResponse = 0.5f;
	
	UPROPERTY(EditDefaultsOnly, Category="Input") UInputMappingContext* FlightMappingContext;
	UPROPERTY(EditDefaultsOnly, Category="Input") UInputAction* PitchAction;
	UPROPERTY(EditDefaultsOnly, Category="Input") UInputAction* RollAction;
	UPROPERTY(EditDefaultsOnly, Category="Input") UInputAction* ThrottleAction;

	bool bOnGround = false;

	UPROPERTY(EditAnywhere, Category="Flight|Ground") float BrakeDecelMS2 = 4.f;
	UPROPERTY(EditAnywhere, Category="Flight|Ground") float RollingFrictionMS2 = 0.8f;

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
