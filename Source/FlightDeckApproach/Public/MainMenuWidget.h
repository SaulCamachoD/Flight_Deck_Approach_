#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "MainMenuWidget.generated.h"

class UMenuButton;
class UTextBlock;

UCLASS()
class FLIGHTDECKAPPROACH_API UMainMenuWidget : public UUserWidget
{
	GENERATED_BODY()

protected:
	virtual void NativeConstruct() override;

	UPROPERTY(meta=(BindWidget)) UMenuButton* PlayButton;
	UPROPERTY(meta=(BindWidget)) UMenuButton* QuitButton;
	UPROPERTY(meta=(BindWidgetOptional)) UTextBlock* BestScoreText;

	// Nivel que carga PLAY
	UPROPERTY(EditAnywhere, Category="Flow") FName GameLevelName = TEXT("Airport");

private:
	UFUNCTION() void OnPlayClicked();
	UFUNCTION() void OnQuitClicked();
};