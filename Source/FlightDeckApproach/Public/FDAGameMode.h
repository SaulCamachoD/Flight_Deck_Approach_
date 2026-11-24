#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "LandingScoreComponent.h"   // FLandingResult
#include "FDAGameMode.generated.h"

class ULandingReportWidget;
class UPauseMenuWidget;

UCLASS()
class FLIGHTDECKAPPROACH_API AFDAGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	// Se llama al inicio y en cada Retry: aquí nos suscribimos al puntaje del avión nuevo
	virtual void RestartPlayer(AController* NewPlayer) override;
	virtual void BeginPlay() override;
	UFUNCTION(BlueprintCallable, Category="Flow") void Retry();
	UFUNCTION(BlueprintCallable, Category="Flow") void GoToMainMenu();
	void TogglePause();
	void ResumeGame();

protected:
	UPROPERTY(EditDefaultsOnly, Category="UI") TSubclassOf<ULandingReportWidget> ReportWidgetClass;
	UPROPERTY(EditDefaultsOnly, Category="UI") float ReportDelaySeconds = 1.5f;
	UPROPERTY(EditDefaultsOnly, Category="UI") TSubclassOf<UPauseMenuWidget> PauseWidgetClass;
	
	// --- Viento ---
	UPROPERTY(EditDefaultsOnly, Category="Wind") bool bRandomWind = true;
	UPROPERTY(EditDefaultsOnly, Category="Wind", meta=(EditCondition="bRandomWind")) float MaxWindKt = 15.f;
	UPROPERTY(EditDefaultsOnly, Category="Wind", meta=(EditCondition="bRandomWind")) float MaxGustKt = 8.f;
	// Ángulo máximo entre el viento y el eje de pista. 90 = nunca viento de cola
	UPROPERTY(EditDefaultsOnly, Category="Wind", meta=(EditCondition="bRandomWind")) float MaxWindAngleDeg = 90.f;

	UPROPERTY(EditDefaultsOnly, Category="Wind", meta=(EditCondition="!bRandomWind")) float FixedWindFromDeg = 270.f;
	UPROPERTY(EditDefaultsOnly, Category="Wind", meta=(EditCondition="!bRandomWind")) float FixedWindKt = 10.f;
	UPROPERTY(EditDefaultsOnly, Category="Wind", meta=(EditCondition="!bRandomWind")) float FixedGustKt = 0.f;

	// Nombre del nivel del menú (ej. MainMenu). Vacío = el botón hace Retry
	UPROPERTY(EditDefaultsOnly, Category="Flow") FName MainMenuLevelName;

private:
	UFUNCTION() void HandleLandingScored(const FLandingResult& Result);
	void ShowReport();
	
	void ConfigureWind();
	void ClosePauseMenu();
	
	UPROPERTY() TObjectPtr<UPauseMenuWidget> PauseWidget;
	UPROPERTY() TObjectPtr<ULandingReportWidget> ReportWidget;
	FLandingResult PendingResult;
	int32 BestScore = 0;   
	FTimerHandle ReportTimer;
};