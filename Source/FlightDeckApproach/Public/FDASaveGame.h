#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
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
	UPROPERTY() EWindPreset WindPreset = EWindPreset::Random;

	static int32 LoadBestScore();
	static void SaveBestScore(int32 Score);

	static EWindPreset LoadWindPreset();
	static void SaveWindPreset(EWindPreset Preset);

private:
	// Carga el archivo, o crea uno nuevo si no existe
	static UFDASaveGame* LoadOrCreate();
	void Write();
};