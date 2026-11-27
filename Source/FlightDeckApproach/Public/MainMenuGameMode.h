#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "MainMenuGameMode.generated.h"

class UUserWidget;

// Nivel del menú: sin avión, solo la UI con el mouse
UCLASS()
class FLIGHTDECKAPPROACH_API AMainMenuGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	AMainMenuGameMode();

protected:
	virtual void BeginPlay() override;

	UPROPERTY(EditDefaultsOnly, Category="UI") TSubclassOf<UUserWidget> MenuWidgetClass;
	UPROPERTY(EditDefaultsOnly, Category="Audio") class USoundBase* MenuMusic;
	UPROPERTY(EditDefaultsOnly, Category="Audio") float MusicFadeInSeconds = 2.f;
	
};