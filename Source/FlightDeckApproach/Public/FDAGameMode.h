#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "LandingScoreComponent.h"   // FLandingResult
#include "FDAGameMode.generated.h"

class ULandingReportWidget;

UCLASS()
class FLIGHTDECKAPPROACH_API AFDAGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	// Se llama al inicio y en cada Retry: aquí nos suscribimos al puntaje del avión nuevo
	virtual void RestartPlayer(AController* NewPlayer) override;

	UFUNCTION(BlueprintCallable, Category="Flow") void Retry();
	UFUNCTION(BlueprintCallable, Category="Flow") void GoToMainMenu();

protected:
	UPROPERTY(EditDefaultsOnly, Category="UI") TSubclassOf<ULandingReportWidget> ReportWidgetClass;
	UPROPERTY(EditDefaultsOnly, Category="UI") float ReportDelaySeconds = 1.5f;

	// Vacío por ahora: el menú principal llega en la Fase 4
	UPROPERTY(EditDefaultsOnly, Category="Flow") FName MainMenuLevelName;

private:
	UFUNCTION() void HandleLandingScored(const FLandingResult& Result);
	void ShowReport();

	UPROPERTY() TObjectPtr<ULandingReportWidget> ReportWidget;
	FLandingResult PendingResult;
	int32 BestScore = 0;   
	FTimerHandle ReportTimer;
};