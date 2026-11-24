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

void UFDASaveGame::SaveBestScore(int32 Score)
{
	if (UFDASaveGame* Save = LoadOrCreate())
	{
		Save->BestScore = Score;   // el resto de datos se conserva
		Save->Write();
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