#include "FDAGameMode.h"
#include "AircraftPawn.h"
#include "LandingReportWidget.h"
#include "WindSubsystem.h"
#include "ILSTransmitter.h"
#include "Kismet/GameplayStatics.h"
#include "FDASaveGame.h"
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

void AFDAGameMode::BeginPlay()
{
	Super::BeginPlay();
	BestScore = UFDASaveGame::LoadBestScore();

	// Si venimos del menú, la entrada quedó en modo UI: devolvérsela al juego
	if (APlayerController* PC = GetWorld()->GetFirstPlayerController())
	{
		PC->SetInputMode(FInputModeGameOnly());
		PC->SetShowMouseCursor(false);
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
	if (bNewBest)
	{
		BestScore = PendingResult.TotalScore;
		UFDASaveGame::SaveBest(PendingResult);  
	}
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

	// El eje de pista es el yaw del transmisor (aterrizamos hacia su +X)
	float RunwayYaw = 0.f;
	if (const AActor* T = UGameplayStatics::GetActorOfClass(this, AILSTransmitter::StaticClass()))
	{
		RunwayYaw = T->GetActorRotation().Yaw;
	}

	switch (UFDASaveGame::LoadWindPreset())
	{
	case EWindPreset::Calm:
		Wind->SetWind(RunwayYaw, 0.f, 0.f);
		break;

	case EWindPreset::Crosswind:
		// Directo de un costado (izquierda o derecha al azar), sin ráfagas
		Wind->SetWind(RunwayYaw + (FMath::RandBool() ? 90.f : -90.f), FMath::FRandRange(10.f, 15.f), 0.f);
		break;

	case EWindPreset::Gusty:
		Wind->SetWind(RunwayYaw + FMath::FRandRange(-45.f, 45.f),
			FMath::FRandRange(12.f, 18.f), FMath::FRandRange(8.f, 12.f));
		break;

	default: // Random: lo que diga BP_FDAGameMode (aleatorio, o fijo para pruebas)
		if (bRandomWind)
		{
			const float From = RunwayYaw + FMath::FRandRange(-MaxWindAngleDeg, MaxWindAngleDeg);
			Wind->SetWind(From, FMath::FRandRange(0.f, MaxWindKt), FMath::FRandRange(0.f, MaxGustKt));
		}
		else
		{
			Wind->SetWind(FixedWindFromDeg, FixedWindKt, FixedGustKt);
		}
		break;
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