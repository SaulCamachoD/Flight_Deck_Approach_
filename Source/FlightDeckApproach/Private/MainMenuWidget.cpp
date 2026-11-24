#include "MainMenuWidget.h"
#include "MenuButton.h"
#include "Components/TextBlock.h"
#include "Components/WidgetSwitcher.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"

void UMainMenuWidget::NativeConstruct()
{
	Super::NativeConstruct();

	PlayButton->OnClicked.AddUniqueDynamic(this, &UMainMenuWidget::OnPlayClicked);
	WeatherButton->OnClicked.AddUniqueDynamic(this, &UMainMenuWidget::OnWeatherClicked);
	QuitButton->OnClicked.AddUniqueDynamic(this, &UMainMenuWidget::OnQuitClicked);

	WindRandomButton->OnClicked.AddUniqueDynamic(this, &UMainMenuWidget::OnWindRandom);
	WindCalmButton->OnClicked.AddUniqueDynamic(this, &UMainMenuWidget::OnWindCalm);
	WindCrossButton->OnClicked.AddUniqueDynamic(this, &UMainMenuWidget::OnWindCross);
	WindGustyButton->OnClicked.AddUniqueDynamic(this, &UMainMenuWidget::OnWindGusty);
	WeatherBackButton->OnClicked.AddUniqueDynamic(this, &UMainMenuWidget::OnWeatherBackClicked);

	MenuSwitcher->SetActiveWidgetIndex(0);
	SelectWind(UFDASaveGame::LoadWindPreset(), false);   // marca la opción guardada

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

void UMainMenuWidget::OnWeatherClicked()
{
	WeatherButton->SetSelected(false);    // que no se quede iluminado al volver
	MenuSwitcher->SetActiveWidgetIndex(1);
}

void UMainMenuWidget::OnWeatherBackClicked()
{
	WeatherBackButton->SetSelected(false);
	MenuSwitcher->SetActiveWidgetIndex(0);
}

void UMainMenuWidget::OnWindRandom() { SelectWind(EWindPreset::Random,    true); }
void UMainMenuWidget::OnWindCalm()   { SelectWind(EWindPreset::Calm,      true); }
void UMainMenuWidget::OnWindCross()  { SelectWind(EWindPreset::Crosswind, true); }
void UMainMenuWidget::OnWindGusty()  { SelectWind(EWindPreset::Gusty,     true); }

void UMainMenuWidget::SelectWind(EWindPreset Preset, bool bSave)
{
	if (bSave) UFDASaveGame::SaveWindPreset(Preset);

	// Solo la opción elegida queda iluminada
	WindRandomButton->SetSelected(Preset == EWindPreset::Random);
	WindCalmButton->SetSelected(Preset == EWindPreset::Calm);
	WindCrossButton->SetSelected(Preset == EWindPreset::Crosswind);
	WindGustyButton->SetSelected(Preset == EWindPreset::Gusty);

	if (WindPresetText)
	{
		const TCHAR* Name = TEXT("RANDOM");
		switch (Preset)
		{
		case EWindPreset::Calm:      Name = TEXT("CALM");  break;
		case EWindPreset::Crosswind: Name = TEXT("XWIND"); break;
		case EWindPreset::Gusty:     Name = TEXT("GUSTY"); break;
		default: break;
		}
		WindPresetText->SetText(FText::FromString(Name));  
	}
}