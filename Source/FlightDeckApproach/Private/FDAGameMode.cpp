#include "FDAGameMode.h"
#include "AircraftPawn.h"
#include "LandingReportWidget.h"
#include "WindSubsystem.h"
#include "ILSTransmitter.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"

void AFDAGameMode::RestartPlayer(AController* NewPlayer)
{
	ConfigureWind();
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

void AFDAGameMode::ConfigureWind()
{
	UWindSubsystem* Wind = GetWorld()->GetSubsystem<UWindSubsystem>();
	if (!Wind) return;

	if (!bRandomWind)
	{
		Wind->SetWind(FixedWindFromDeg, FixedWindKt, FixedGustKt);
	}
	else
	{
		// El eje de pista es el yaw del transmisor (aterrizamos hacia su +X)
		float RunwayYaw = 0.f;
		if (const AActor* T = UGameplayStatics::GetActorOfClass(this, AILSTransmitter::StaticClass()))
		{
			RunwayYaw = T->GetActorRotation().Yaw;
		}
		const float From = RunwayYaw + FMath::FRandRange(-MaxWindAngleDeg, MaxWindAngleDeg);
		Wind->SetWind(From, FMath::FRandRange(0.f, MaxWindKt), FMath::FRandRange(0.f, MaxGustKt));
	}

	UE_LOG(LogTemp, Warning, TEXT("WIND %03.0f / %.0f KT  gust +%.0f"),
		FRotator::ClampAxis(Wind->GetBaseFromDeg()), Wind->GetBaseSpeedKt(), Wind->GetGustKt());
	
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