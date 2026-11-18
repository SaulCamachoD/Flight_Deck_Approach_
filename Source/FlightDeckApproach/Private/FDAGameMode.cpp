#include "FDAGameMode.h"
#include "AircraftPawn.h"
#include "LandingReportWidget.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"

void AFDAGameMode::RestartPlayer(AController* NewPlayer)
{
	Super::RestartPlayer(NewPlayer);

	const AAircraftPawn* Aircraft = NewPlayer ? Cast<AAircraftPawn>(NewPlayer->GetPawn()) : nullptr;
	if (ULandingScoreComponent* Score = Aircraft ? Aircraft->GetLandingScore() : nullptr)
	{
		Score->OnLandingScored.AddUniqueDynamic(this, &AFDAGameMode::HandleLandingScored);
	}
}

void AFDAGameMode::HandleLandingScored(const FLandingResult& Result)
{
	// Una pausa corta para ver el avión detenerse (o estrellarse) antes del reporte
	PendingResult = Result;
	GetWorldTimerManager().SetTimer(ReportTimer, this, &AFDAGameMode::ShowReport, ReportDelaySeconds, false);
}

void AFDAGameMode::ShowReport()
{
	APlayerController* PC = GetWorld()->GetFirstPlayerController();
	if (!PC || !ReportWidgetClass) return;

	ReportWidget = CreateWidget<ULandingReportWidget>(PC, ReportWidgetClass);
	if (!ReportWidget) return;

	const bool bNewBest = !PendingResult.bCrashed && PendingResult.TotalScore > BestScore;
	if (bNewBest) BestScore = PendingResult.TotalScore;
	ReportWidget->SetResult(PendingResult, bNewBest);
	
	if (AAircraftPawn* Aircraft = Cast<AAircraftPawn>(PC->GetPawn()))
	{
		Aircraft->SetInstrumentsVisible(false);
	}
	
	ReportWidget->AddToViewport(10);   // por encima del HUD

	PC->SetInputMode(FInputModeUIOnly());
	PC->SetShowMouseCursor(true);
}

void AFDAGameMode::Retry()
{
	GetWorldTimerManager().ClearTimer(ReportTimer);
	if (ReportWidget)
	{
		ReportWidget->RemoveFromParent();
		ReportWidget = nullptr;
	}

	APlayerController* PC = GetWorld()->GetFirstPlayerController();
	if (!PC) return;

	if (APawn* OldPawn = PC->GetPawn())
	{
		PC->UnPossess();
		OldPawn->Destroy();   // su EndPlay quita el HUD viejo
	}

	RestartPlayer(PC);        // avión nuevo en el PlayerStart

	PC->SetInputMode(FInputModeGameOnly());
	PC->SetShowMouseCursor(false);
}

void AFDAGameMode::GoToMainMenu()
{
	if (MainMenuLevelName.IsNone())
	{
		UE_LOG(LogTemp, Warning, TEXT("FDAGameMode: MainMenuLevelName vacío, hago Retry"));
		Retry();
		return;
	}
	UGameplayStatics::OpenLevel(this, MainMenuLevelName);
}