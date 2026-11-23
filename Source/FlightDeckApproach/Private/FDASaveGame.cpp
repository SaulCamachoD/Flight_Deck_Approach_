#include "FDASaveGame.h"
#include "Kismet/GameplayStatics.h"

namespace
{
	// Archivo en Saved/SaveGames/FlightDeckApproach.sav
	const TCHAR* SlotName = TEXT("FlightDeckApproach");
}

int32 UFDASaveGame::LoadBestScore()
{
	if (const UFDASaveGame* Save = Cast<UFDASaveGame>(UGameplayStatics::LoadGameFromSlot(SlotName, 0)))
	{
		return Save->BestScore;
	}
	return 0;   // primera vez: no hay archivo
}

void UFDASaveGame::SaveBestScore(int32 Score)
{
	if (UFDASaveGame* Save = Cast<UFDASaveGame>(UGameplayStatics::CreateSaveGameObject(StaticClass())))
	{
		Save->BestScore = Score;
		UGameplayStatics::SaveGameToSlot(Save, SlotName, 0);
	}
}