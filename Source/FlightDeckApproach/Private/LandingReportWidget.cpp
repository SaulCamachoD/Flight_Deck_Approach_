#include "LandingReportWidget.h"
#include "FDAGameMode.h"
#include "Components/TextBlock.h"
#include "Components/Button.h"
#include "Components/PanelWidget.h"
#include "Components/ProgressBar.h"

void ULandingReportWidget::NativeConstruct()
{
	Super::NativeConstruct();
	RetryButton->OnClicked.AddUniqueDynamic(this, &ULandingReportWidget::OnRetryClicked);
	MenuButton->OnClicked.AddUniqueDynamic(this, &ULandingReportWidget::OnMenuClicked);
}

void ULandingReportWidget::OnRetryClicked()
{
	if (AFDAGameMode* GM = GetWorld()->GetAuthGameMode<AFDAGameMode>()) GM->Retry();
}

void ULandingReportWidget::OnMenuClicked()
{
	if (AFDAGameMode* GM = GetWorld()->GetAuthGameMode<AFDAGameMode>()) GM->GoToMainMenu();
}

FLinearColor ULandingReportWidget::ColorFor(float Ratio) const
{
	if (Ratio >= 0.9f) return PerfectColor;
	if (Ratio >= 0.7f) return GoodColor;
	if (Ratio >= 0.4f) return FairColor;
	return BadColor;
}

FString ULandingReportWidget::WordFor(float Ratio)
{
	if (Ratio >= 0.9f) return TEXT("PERFECT");
	if (Ratio >= 0.7f) return TEXT("GOOD");
	if (Ratio >= 0.4f) return TEXT("FAIR");
	return TEXT("POOR");
}

FString ULandingReportWidget::LetterFor(const FString& Grade)
{
	if (Grade == TEXT("EXCELLENT")) return TEXT("A");
	if (Grade == TEXT("GOOD"))      return TEXT("B");
	if (Grade == TEXT("FAIR"))      return TEXT("C");
	if (Grade == TEXT("POOR"))      return TEXT("D");
	return TEXT("F");   // CRASH
}

FString ULandingReportWidget::CrashReasonToString(ECrashReason Reason)
{
	switch (Reason)
	{
	case ECrashReason::OffRunway:       return TEXT("TOUCHDOWN OFF RUNWAY");
	case ECrashReason::HardLanding:     return TEXT("HARD LANDING");
	case ECrashReason::ExcessiveBank:   return TEXT("EXCESSIVE BANK");
	case ECrashReason::NoseFirst:       return TEXT("NOSE-FIRST TOUCHDOWN");
	case ECrashReason::RunwayExcursion: return TEXT("RUNWAY EXCURSION");
	case ECrashReason::Collision:       return TEXT("TERRAIN COLLISION");
	default:                            return TEXT("CRASH");
	}
}

void ULandingReportWidget::SetRow(UTextBlock* Value, UTextBlock* Rating, UProgressBar* Bar, UTextBlock* Pts,
                                  const FString& ValueStr, float Ratio, int32 Points) const
{
	const FLinearColor C = ColorFor(Ratio);

	if (Value) Value->SetText(FText::FromString(ValueStr));
	if (Rating)
	{
		Rating->SetText(FText::FromString(WordFor(Ratio)));
		Rating->SetColorAndOpacity(C);
	}
	if (Bar)
	{
		Bar->SetPercent(Ratio);
		Bar->SetFillColorAndOpacity(C);
	}
	if (Pts) Pts->SetText(FText::FromString(FString::Printf(TEXT("%d PTS"), Points)));
}

void ULandingReportWidget::SetResult(const FLandingResult& R, bool bNewBest)
{
	const bool bCrash = R.bCrashed;

	TitleText->SetText(FText::FromString(bCrash ? TEXT("CRASH REPORT") : TEXT("LANDING REPORT")));
	TitleText->SetColorAndOpacity(AccentColor);

	// Emblema: letra coloreada por la nota
	const float Total01 = R.TotalScore / 1000.f;
	GradeText->SetText(FText::FromString(LetterFor(R.Grade)));
	GradeText->SetColorAndOpacity(bCrash ? BadColor : ColorFor(Total01));

	ScoreText->SetText(FText::Format(NSLOCTEXT("Report", "Score", "SCORE {0}"), FText::AsNumber(R.TotalScore)));
	ScoreText->SetColorAndOpacity(AccentColor);

	if (BestBadge)
	{
		BestBadge->SetVisibility(bNewBest ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Collapsed);
	}

	if (CrashReasonText)
	{
		CrashReasonText->SetText(FText::FromString(CrashReasonToString(R.CrashReason)));
		CrashReasonText->SetColorAndOpacity(BadColor);
		CrashReasonText->SetVisibility(bCrash ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Collapsed);
	}

	if (DetailsBox)
	{
		DetailsBox->SetVisibility(bCrash ? ESlateVisibility::Collapsed : ESlateVisibility::SelfHitTestInvisible);
	}
	if (bCrash) return;

	// Mismo orden que la referencia
	SetRow(ZoneValue, ZoneRating, ZoneBar, ZonePts,
		FString::Printf(TEXT("Touchdown distance from aim point: %.0f M %s"),
			FMath::Abs(R.DistanceFromAimPointM), R.DistanceFromAimPointM >= 0.f ? TEXT("LONG") : TEXT("SHORT")),
		R.ZoneRatio, R.ZonePoints);

	SetRow(CenterValue, CenterRating, CenterBar, CenterPts,
		FString::Printf(TEXT("Centerline deviation: %.1f M %s"),
			FMath::Abs(R.CenterlineOffsetM), R.CenterlineOffsetM >= 0.f ? TEXT("R") : TEXT("L")),
		R.CenterlineRatio, R.CenterlinePoints);

	SetRow(SinkValue, SinkRating, SinkBar, SinkPts,
		FString::Printf(TEXT("Vertical speed at touchdown: -%.0f FPM"), R.SinkRateFPM),
		R.SinkRatio, R.SinkRatePoints);

	SetRow(TrackingValue, TrackingRating, TrackingBar, TrackingPts,
		FString::Printf(TEXT("ILS tracking: LOC %.0f%%  GS %.0f%%"), R.LocalizerTrackingPct, R.GlideslopeTrackingPct),
		R.TrackingRatio, R.TrackingPoints);

	SetRow(SpeedValue, SpeedRating, SpeedBar, SpeedPts,
		FString::Printf(TEXT("Touchdown speed: %.0f KT"), R.TouchdownSpeedKt),
		R.SpeedRatio, R.SpeedPoints);
}