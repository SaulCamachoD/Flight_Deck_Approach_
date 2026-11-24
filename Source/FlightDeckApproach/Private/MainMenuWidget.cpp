#include "MainMenuWidget.h"
#include "MenuButton.h"
#include "LandingReportWidget.h"   // LetterFor
#include "Components/TextBlock.h"
#include "Components/WidgetSwitcher.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"

namespace
{
	// Misma escala de colores que el Landing Report
	FLinearColor RatioColor(float Ratio)
	{
		if (Ratio >= 0.9f) return FLinearColor(0.3f, 1.f, 0.45f);   // verde
		if (Ratio >= 0.7f) return FLinearColor(1.f, 0.85f, 0.2f);    // amarillo
		if (Ratio >= 0.4f) return FLinearColor(1.f, 0.5f, 0.1f);     // naranja
		return FLinearColor(1.f, 0.15f, 0.1f);                       // rojo
	}

	void SetPts(UTextBlock* Text, int32 Points, float Ratio)
	{
		if (!Text) return;
		Text->SetText(FText::FromString(FString::Printf(TEXT("%d PTS"), Points)));
		Text->SetColorAndOpacity(RatioColor(Ratio));
	}
}

void UMainMenuWidget::NativeConstruct()
{
	Super::NativeConstruct();

	PlayButton->OnClicked.AddUniqueDynamic(this, &UMainMenuWidget::OnPlayClicked);
	WeatherButton->OnClicked.AddUniqueDynamic(this, &UMainMenuWidget::OnWeatherClicked);
	ScoresButton->OnClicked.AddUniqueDynamic(this, &UMainMenuWidget::OnScoresClicked);
	QuitButton->OnClicked.AddUniqueDynamic(this, &UMainMenuWidget::OnQuitClicked);

	WindRandomButton->OnClicked.AddUniqueDynamic(this, &UMainMenuWidget::OnWindRandom);
	WindCalmButton->OnClicked.AddUniqueDynamic(this, &UMainMenuWidget::OnWindCalm);
	WindCrossButton->OnClicked.AddUniqueDynamic(this, &UMainMenuWidget::OnWindCross);
	WindGustyButton->OnClicked.AddUniqueDynamic(this, &UMainMenuWidget::OnWindGusty);
	WeatherBackButton->OnClicked.AddUniqueDynamic(this, &UMainMenuWidget::OnWeatherBackClicked);

	ScoresResetButton->OnClicked.AddUniqueDynamic(this, &UMainMenuWidget::OnScoresResetClicked);
	ScoresBackButton->OnClicked.AddUniqueDynamic(this, &UMainMenuWidget::OnScoresBackClicked);

	MenuSwitcher->SetActiveWidgetIndex(0);
	SelectWind(UFDASaveGame::LoadWindPreset(), false);   // marca la opción guardada
	RefreshBest();                                        // franja + pantalla de récords
}

void UMainMenuWidget::OnPlayClicked()
{
	UGameplayStatics::OpenLevel(this, GameLevelName);
}

void UMainMenuWidget::OnQuitClicked()
{
	UKismetSystemLibrary::QuitGame(this, GetOwningPlayer(), EQuitPreference::Quit, false);
}

// ---------- WEATHER & WIND ----------

void UMainMenuWidget::OnWeatherClicked()
{
	WeatherButton->SetSelected(false);    // que no se quede iluminado al volver
	MenuSwitcher->SetActiveWidgetIndex(1);
}

void UMainMenuWidget::OnWeatherBackClicked()
{
	WeatherBackButton->SetSelected(false);
	MenuSwitcher->SetActiveWidgetIndex(0);
}

void UMainMenuWidget::OnWindRandom() { SelectWind(EWindPreset::Random,    true); }
void UMainMenuWidget::OnWindCalm()   { SelectWind(EWindPreset::Calm,      true); }
void UMainMenuWidget::OnWindCross()  { SelectWind(EWindPreset::Crosswind, true); }
void UMainMenuWidget::OnWindGusty()  { SelectWind(EWindPreset::Gusty,     true); }

void UMainMenuWidget::SelectWind(EWindPreset Preset, bool bSave)
{
	if (bSave) UFDASaveGame::SaveWindPreset(Preset);

	// Solo la opción elegida queda iluminada
	WindRandomButton->SetSelected(Preset == EWindPreset::Random);
	WindCalmButton->SetSelected(Preset == EWindPreset::Calm);
	WindCrossButton->SetSelected(Preset == EWindPreset::Crosswind);
	WindGustyButton->SetSelected(Preset == EWindPreset::Gusty);

	if (WindPresetText)
	{
		const TCHAR* Name = TEXT("RANDOM");
		switch (Preset)
		{
		case EWindPreset::Calm:      Name = TEXT("CALM");  break;
		case EWindPreset::Crosswind: Name = TEXT("XWIND"); break;
		case EWindPreset::Gusty:     Name = TEXT("GUSTY"); break;
		default: break;
		}
		WindPresetText->SetText(FText::FromString(Name));   // la separación la da el padding del slot
	}
}

// ---------- BEST SCORES ----------

void UMainMenuWidget::OnScoresClicked()
{
	ScoresButton->SetSelected(false);
	RefreshBest();
	MenuSwitcher->SetActiveWidgetIndex(2);
}

void UMainMenuWidget::OnScoresBackClicked()
{
	ScoresBackButton->SetSelected(false);
	DisarmReset();
	MenuSwitcher->SetActiveWidgetIndex(0);
}

void UMainMenuWidget::OnScoresResetClicked()
{
	// Dos clics: el primero pide confirmación, el segundo borra
	if (!bResetArmed)
	{
		bResetArmed = true;
		ScoresResetButton->SetLabel(INVTEXT("CONFIRM RESET?"));
		return;
	}
	UFDASaveGame::ResetBest();
	DisarmReset();
	RefreshBest();
}

void UMainMenuWidget::DisarmReset()
{
	bResetArmed = false;
	ScoresResetButton->SetLabel(INVTEXT("RESET"));
}

void UMainMenuWidget::RefreshBest()
{
	FLandingResult R;
	FDateTime Date;
	const bool bHasBest = UFDASaveGame::LoadBest(R, Date);

	// Franja inferior
	if (BestScoreText)
	{
		BestScoreText->SetText(FText::FromString(bHasBest ? FString::FromInt(R.TotalScore) : FString(TEXT("---"))));
	}

	// Encabezado de la pantalla de récords
	if (BestGradeText)
	{
		BestGradeText->SetText(FText::FromString(bHasBest ? ULandingReportWidget::LetterFor(R.Grade) : FString(TEXT("-"))));
		BestGradeText->SetColorAndOpacity(bHasBest ? RatioColor(R.TotalScore / 1000.f) : FLinearColor::Gray);
	}
	if (BestTotalText)
	{
		BestTotalText->SetText(bHasBest
			? FText::Format(INVTEXT("SCORE {0}"), FText::AsNumber(R.TotalScore))
			: INVTEXT("NO LANDING YET"));
	}
	if (BestDateText)
	{
		BestDateText->SetText(bHasBest ? FText::AsDate(Date) : FText::GetEmpty());
	}

	// Desglose: oculto si aún no hay récord
	if (BestDetails)
	{
		BestDetails->SetVisibility(bHasBest ? ESlateVisibility::SelfHitTestInvisible : ESlateVisibility::Collapsed);
	}
	if (!bHasBest) return;

	SetPts(BestTrackingPts, R.TrackingPoints,   R.TrackingRatio);
	SetPts(BestSinkPts,     R.SinkRatePoints,   R.SinkRatio);
	SetPts(BestZonePts,     R.ZonePoints,       R.ZoneRatio);
	SetPts(BestCenterPts,   R.CenterlinePoints, R.CenterlineRatio);
	SetPts(BestSpeedPts,    R.SpeedPoints,      R.SpeedRatio);
}