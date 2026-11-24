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
	UPROPERTY(meta=(BindWidget)) UMenuButton* QuitButton;
	UPROPERTY(meta=(BindWidget)) UWidgetSwitcher* MenuSwitcher;   // 0 = principal, 1 = viento

	// --- Pantalla WEATHER & WIND ---
	UPROPERTY(meta=(BindWidget)) UMenuButton* WindRandomButton;
	UPROPERTY(meta=(BindWidget)) UMenuButton* WindCalmButton;
	UPROPERTY(meta=(BindWidget)) UMenuButton* WindCrossButton;
	UPROPERTY(meta=(BindWidget)) UMenuButton* WindGustyButton;
	UPROPERTY(meta=(BindWidget)) UMenuButton* WeatherBackButton;

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

	void SelectWind(EWindPreset Preset, bool bSave);
};