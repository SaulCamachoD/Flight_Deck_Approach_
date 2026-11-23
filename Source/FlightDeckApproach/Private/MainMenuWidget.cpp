#include "MainMenuWidget.h"
#include "FDASaveGame.h"
#include "MenuButton.h"
#include "Components/TextBlock.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"

void UMainMenuWidget::NativeConstruct()
{
	Super::NativeConstruct();

	PlayButton->OnClicked.AddUniqueDynamic(this, &UMainMenuWidget::OnPlayClicked);
	QuitButton->OnClicked.AddUniqueDynamic(this, &UMainMenuWidget::OnQuitClicked);

	// Solo el número: la etiqueta "BEST" está en el WBP
	if (BestScoreText)
	{
		const int32 Best = UFDASaveGame::LoadBestScore();
		BestScoreText->SetText(FText::FromString(Best > 0 ? FString::FromInt(Best) : FString(TEXT("---"))));
	}
}

void UMainMenuWidget::OnPlayClicked()
{
	UGameplayStatics::OpenLevel(this, GameLevelName);
}

void UMainMenuWidget::OnQuitClicked()
{
	UKismetSystemLibrary::QuitGame(this, GetOwningPlayer(), EQuitPreference::Quit, false);
}