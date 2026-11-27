#include "MainMenuGameMode.h"
#include "Blueprint/UserWidget.h"
#include "Kismet/GameplayStatics.h"
#include "Components/AudioComponent.h"

AMainMenuGameMode::AMainMenuGameMode()
{
	DefaultPawnClass = nullptr;   // en el menú no se genera ningún pawn
}

void AMainMenuGameMode::BeginPlay()
{
	Super::BeginPlay();
	// Se detiene sola al cargar el nivel del juego
	if (MenuMusic)
	{
		if (UAudioComponent* Music = UGameplayStatics::CreateSound2D(this, MenuMusic))
		{
			Music->FadeIn(MusicFadeInSeconds);
		}
	}
	APlayerController* PC = GetWorld()->GetFirstPlayerController();
	if (!PC || !MenuWidgetClass) return;

	if (UUserWidget* Menu = CreateWidget<UUserWidget>(PC, MenuWidgetClass))
	{
		Menu->AddToViewport();

		FInputModeUIOnly Mode;
		Mode.SetWidgetToFocus(Menu->TakeWidget());
		PC->SetInputMode(Mode);
		PC->SetShowMouseCursor(true);
	}
}