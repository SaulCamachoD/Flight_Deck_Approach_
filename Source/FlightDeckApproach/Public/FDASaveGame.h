// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "FDASaveGame.generated.h"

/**
 * 
 */
UCLASS()
class FLIGHTDECKAPPROACH_API UFDASaveGame : public USaveGame
{
	GENERATED_BODY()

public:
	UPROPERTY() int32 BestScore = 0;

	static int32 LoadBestScore();
	static void SaveBestScore(int32 Score);
	
};
