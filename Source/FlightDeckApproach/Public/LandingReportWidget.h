#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "LandingScoreComponent.h"   // FLandingResult
#include "LandingReportWidget.generated.h"

class UTextBlock;
class UButton;
class UPanelWidget;
class UProgressBar;

UCLASS()
class FLIGHTDECKAPPROACH_API ULandingReportWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	void SetResult(const FLandingResult& Result, bool bNewBest);
	static FString LetterFor(const FString& Grade);

protected:
	virtual void NativeConstruct() override;

	// --- Obligatorios ---
	UPROPERTY(meta=(BindWidget)) UTextBlock* TitleText;
	UPROPERTY(meta=(BindWidget)) UTextBlock* GradeText;    // la letra del emblema
	UPROPERTY(meta=(BindWidget)) UTextBlock* ScoreText;
	UPROPERTY(meta=(BindWidget)) UButton* RetryButton;
	UPROPERTY(meta=(BindWidget)) UButton* MenuButton;

	// --- Opcionales ---
	UPROPERTY(meta=(BindWidgetOptional)) UTextBlock* CrashReasonText;
	UPROPERTY(meta=(BindWidgetOptional)) UPanelWidget* DetailsBox;
	UPROPERTY(meta=(BindWidgetOptional)) UWidget* BestBadge;

	// --- Filas: frase + calificación + barra (+ puntos opcional) ---
	UPROPERTY(meta=(BindWidgetOptional)) UTextBlock* ZoneValue;
	UPROPERTY(meta=(BindWidgetOptional)) UTextBlock* ZoneRating;
	UPROPERTY(meta=(BindWidgetOptional)) UProgressBar* ZoneBar;
	UPROPERTY(meta=(BindWidgetOptional)) UTextBlock* ZonePts;

	UPROPERTY(meta=(BindWidgetOptional)) UTextBlock* CenterValue;
	UPROPERTY(meta=(BindWidgetOptional)) UTextBlock* CenterRating;
	UPROPERTY(meta=(BindWidgetOptional)) UProgressBar* CenterBar;
	UPROPERTY(meta=(BindWidgetOptional)) UTextBlock* CenterPts;

	UPROPERTY(meta=(BindWidgetOptional)) UTextBlock* SinkValue;
	UPROPERTY(meta=(BindWidgetOptional)) UTextBlock* SinkRating;
	UPROPERTY(meta=(BindWidgetOptional)) UProgressBar* SinkBar;
	UPROPERTY(meta=(BindWidgetOptional)) UTextBlock* SinkPts;

	UPROPERTY(meta=(BindWidgetOptional)) UTextBlock* TrackingValue;
	UPROPERTY(meta=(BindWidgetOptional)) UTextBlock* TrackingRating;
	UPROPERTY(meta=(BindWidgetOptional)) UProgressBar* TrackingBar;
	UPROPERTY(meta=(BindWidgetOptional)) UTextBlock* TrackingPts;

	UPROPERTY(meta=(BindWidgetOptional)) UTextBlock* SpeedValue;
	UPROPERTY(meta=(BindWidgetOptional)) UTextBlock* SpeedRating;
	UPROPERTY(meta=(BindWidgetOptional)) UProgressBar* SpeedBar;
	UPROPERTY(meta=(BindWidgetOptional)) UTextBlock* SpeedPts;

	// --- Paleta de la referencia ---
	UPROPERTY(EditAnywhere, Category="Colors") FLinearColor AccentColor = FLinearColor(0.55f, 0.85f, 1.f);   // cian
	UPROPERTY(EditAnywhere, Category="Colors") FLinearColor PerfectColor = FLinearColor(0.3f, 1.f, 0.45f);  // verde
	UPROPERTY(EditAnywhere, Category="Colors") FLinearColor GoodColor   = FLinearColor(1.f, 0.85f, 0.2f);   // amarillo
	UPROPERTY(EditAnywhere, Category="Colors") FLinearColor FairColor   = FLinearColor(1.f, 0.5f, 0.1f);    // naranja
	UPROPERTY(EditAnywhere, Category="Colors") FLinearColor BadColor    = FLinearColor(1.f, 0.15f, 0.1f);   // rojo

private:
	UFUNCTION() void OnRetryClicked();
	UFUNCTION() void OnMenuClicked();

	void SetRow(UTextBlock* Value, UTextBlock* Rating, UProgressBar* Bar, UTextBlock* Pts,
	            const FString& ValueStr, float Ratio, int32 Points) const;
	FLinearColor ColorFor(float Ratio) const;
	static FString WordFor(float Ratio);
	static FString CrashReasonToString(ECrashReason Reason);
};