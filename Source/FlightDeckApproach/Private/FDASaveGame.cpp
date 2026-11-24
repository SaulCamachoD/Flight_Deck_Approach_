#include "FDASaveGame.h"
#include "Kismet/GameplayStatics.h"

namespace
{
	// Archivo en Saved/SaveGames/FlightDeckApproach.sav
	const TCHAR* SlotName = TEXT("FlightDeckApproach");
}

UFDASaveGame* UFDASaveGame::LoadOrCreate()
{
	if (UFDASaveGame* Save = Cast<UFDASaveGame>(UGameplayStatics::LoadGameFromSlot(SlotName, 0)))
	{
		return Save;
	}
	return Cast<UFDASaveGame>(UGameplayStatics::CreateSaveGameObject(StaticClass()));
}

void UFDASaveGame::Write()
{
	UGameplayStatics::SaveGameToSlot(this, SlotName, 0);
}

int32 UFDASaveGame::LoadBestScore()
{
	const UFDASaveGame* Save = LoadOrCreate();
	return Save ? Save->BestScore : 0;
}

void UFDASaveGame::SaveBest(const FLandingResult& Result)
{
	if (UFDASaveGame* Save = LoadOrCreate())
	{
		Save->BestScore  = Result.TotalScore;
		Save->BestResult = Result;
		Save->BestDate   = FDateTime::Now();
		Save->Write();   // el preset de viento se conserva
	}
}

EWindPreset UFDASaveGame::LoadWindPreset()
{
	const UFDASaveGame* Save = LoadOrCreate();
	return Save ? Save->WindPreset : EWindPreset::Random;
}

void UFDASaveGame::SaveWindPreset(EWindPreset Preset)
{
	if (UFDASaveGame* Save = LoadOrCreate())
	{
		Save->WindPreset = Preset;
		Save->Write();
	}
}

bool UFDASaveGame::LoadBest(FLandingResult& OutResult, FDateTime& OutDate)
{
	const UFDASaveGame* Save = LoadOrCreate();
	if (!Save || Save->BestScore <= 0) return false;

	OutResult = Save->BestResult;
	OutDate   = Save->BestDate;
	return true;
}

void UFDASaveGame::ResetBest()
{
	if (UFDASaveGame* Save = LoadOrCreate())
	{
		Save->BestScore  = 0;
		Save->BestResult = FLandingResult();
		Save->BestDate   = FDateTime();
		Save->Write();
	}
}