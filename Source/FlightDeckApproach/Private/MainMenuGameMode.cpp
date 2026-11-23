#include "MainMenuGameMode.h"
#include "Blueprint/UserWidget.h"

AMainMenuGameMode::AMainMenuGameMode()
{
	DefaultPawnClass = nullptr;   // en el menú no se genera ningún pawn
}

void AMainMenuGameMode::BeginPlay()
{
	Super::BeginPlay();

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