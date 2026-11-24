#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "LandingScoreComponent.h"   // FLandingResult
#include "FDASaveGame.generated.h"

UENUM(BlueprintType)
enum class EWindPreset : uint8
{
	Random,
	Calm,
	Crosswind,
	Gusty
};

// Lo que sobrevive al cerrar el juego
UCLASS()
class FLIGHTDECKAPPROACH_API UFDASaveGame : public USaveGame
{
	GENERATED_BODY()

public:
	UPROPERTY() int32 BestScore = 0;
	UPROPERTY() FLandingResult BestResult;   // desglose completo del mejor aterrizaje
	UPROPERTY() FDateTime BestDate;
	UPROPERTY() EWindPreset WindPreset = EWindPreset::Random;

	static int32 LoadBestScore();
	static bool LoadBest(FLandingResult& OutResult, FDateTime& OutDate);   // false = todavía no hay
	static void SaveBest(const FLandingResult& Result);
	static void ResetBest();

	static EWindPreset LoadWindPreset();
	static void SaveWindPreset(EWindPreset Preset);

private:
	// Carga el archivo, o crea uno nuevo si no existe
	static UFDASaveGame* LoadOrCreate();
	void Write();
};