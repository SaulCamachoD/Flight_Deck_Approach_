#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "FDASaveGame.h"   // EWindPreset
#include "MainMenuWidget.generated.h"

class UMenuButton;
class UTextBlock;
class UWidgetSwitcher;

UCLASS()
class FLIGHTDECKAPPROACH_API UMainMenuWidget : public UUserWidget
{
	GENERATED_BODY()

protected:
	virtual void NativeConstruct() override;

	// --- Pantalla principal ---
	UPROPERTY(meta=(BindWidget)) UMenuButton* PlayButton;
	UPROPERTY(meta=(BindWidget)) UMenuButton* WeatherButton;
	UPROPERTY(meta=(BindWidget)) UMenuButton* ScoresButton;
	UPROPERTY(meta=(BindWidget)) UMenuButton* QuitButton;
	UPROPERTY(meta=(BindWidget)) UWidgetSwitcher* MenuSwitcher;   // 0 = principal, 1 = viento, 2 = récords

	// --- Pantalla WEATHER & WIND ---
	UPROPERTY(meta=(BindWidget)) UMenuButton* WindRandomButton;
	UPROPERTY(meta=(BindWidget)) UMenuButton* WindCalmButton;
	UPROPERTY(meta=(BindWidget)) UMenuButton* WindCrossButton;
	UPROPERTY(meta=(BindWidget)) UMenuButton* WindGustyButton;
	UPROPERTY(meta=(BindWidget)) UMenuButton* WeatherBackButton;

	// --- Pantalla BEST SCORES ---
	UPROPERTY(meta=(BindWidget)) UMenuButton* ScoresResetButton;
	UPROPERTY(meta=(BindWidget)) UMenuButton* ScoresBackButton;
	UPROPERTY(meta=(BindWidgetOptional)) UTextBlock* BestGradeText;
	UPROPERTY(meta=(BindWidgetOptional)) UTextBlock* BestTotalText;
	UPROPERTY(meta=(BindWidgetOptional)) UTextBlock* BestDateText;
	UPROPERTY(meta=(BindWidgetOptional)) UWidget* BestDetails;   // se oculta si no hay récord
	UPROPERTY(meta=(BindWidgetOptional)) UTextBlock* BestTrackingPts;
	UPROPERTY(meta=(BindWidgetOptional)) UTextBlock* BestSinkPts;
	UPROPERTY(meta=(BindWidgetOptional)) UTextBlock* BestZonePts;
	UPROPERTY(meta=(BindWidgetOptional)) UTextBlock* BestCenterPts;
	UPROPERTY(meta=(BindWidgetOptional)) UTextBlock* BestSpeedPts;

	// --- Franja inferior ---
	UPROPERTY(meta=(BindWidgetOptional)) UTextBlock* BestScoreText;
	UPROPERTY(meta=(BindWidgetOptional)) UTextBlock* WindPresetText;

	// Nivel que carga START APPROACH
	UPROPERTY(EditAnywhere, Category="Flow") FName GameLevelName = TEXT("Airport");

private:
	UFUNCTION() void OnPlayClicked();
	UFUNCTION() void OnQuitClicked();
	UFUNCTION() void OnWeatherClicked();
	UFUNCTION() void OnWeatherBackClicked();
	UFUNCTION() void OnWindRandom();
	UFUNCTION() void OnWindCalm();
	UFUNCTION() void OnWindCross();
	UFUNCTION() void OnWindGusty();
	UFUNCTION() void OnScoresClicked();
	UFUNCTION() void OnScoresBackClicked();
	UFUNCTION() void OnScoresResetClicked();

	void SelectWind(EWindPreset Preset, bool bSave);
	void RefreshBest();
	void DisarmReset();

	bool bResetArmed = false;
};