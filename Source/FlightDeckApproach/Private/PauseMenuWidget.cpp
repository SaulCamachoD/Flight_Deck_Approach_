#include "PauseMenuWidget.h"
#include "MenuButton.h"
#include "FDAGameMode.h"

void UPauseMenuWidget::NativeConstruct()
{
	Super::NativeConstruct();
	SetIsFocusable(true);   // necesario para recibir Esc/P con el foco

	ResumeButton->OnClicked.AddUniqueDynamic(this, &UPauseMenuWidget::OnResumeClicked);
	RetryButton->OnClicked.AddUniqueDynamic(this, &UPauseMenuWidget::OnRetryClicked);
	MainMenuButton->OnClicked.AddUniqueDynamic(this, &UPauseMenuWidget::OnMainMenuClicked);
}

FReply UPauseMenuWidget::NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent)
{
	const FKey Key = InKeyEvent.GetKey();
	const bool bResumeKey = Key == EKeys::P || Key == EKeys::SpaceBar || Key == EKeys::Escape;
	if (bResumeKey && !InKeyEvent.IsRepeat())
	{
		OnResumeClicked();
		return FReply::Handled();
	}
	return Super::NativeOnKeyDown(InGeometry, InKeyEvent);
}

AFDAGameMode* UPauseMenuWidget::GetFDAGameMode() const
{
	return GetWorld() ? GetWorld()->GetAuthGameMode<AFDAGameMode>() : nullptr;
}

void UPauseMenuWidget::OnResumeClicked()
{
	if (AFDAGameMode* GM = GetFDAGameMode()) GM->ResumeGame();
}

void UPauseMenuWidget::OnRetryClicked()
{
	if (AFDAGameMode* GM = GetFDAGameMode()) GM->Retry();          // Retry quita la pausa
}

void UPauseMenuWidget::OnMainMenuClicked()
{
	if (AFDAGameMode* GM = GetFDAGameMode()) GM->GoToMainMenu();
}