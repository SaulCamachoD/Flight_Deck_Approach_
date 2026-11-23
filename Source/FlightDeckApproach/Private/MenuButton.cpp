#include "MenuButton.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"

void UMenuButton::NativePreConstruct()
{
	Super::NativePreConstruct();

	// También corre en el diseñador: así ves el texto real de cada botón en WBP_MainMenu
	if (Label) Label->SetText(Text);
	SetHighlight(false);
}

void UMenuButton::NativeConstruct()
{
	Super::NativeConstruct();

	Button->OnClicked.AddUniqueDynamic(this, &UMenuButton::HandleClicked);
	Button->OnHovered.AddUniqueDynamic(this, &UMenuButton::HandleHovered);
	Button->OnUnhovered.AddUniqueDynamic(this, &UMenuButton::HandleUnhovered);
}

void UMenuButton::HandleClicked()   { OnClicked.Broadcast(); }
void UMenuButton::HandleHovered()   { SetHighlight(true); }
void UMenuButton::HandleUnhovered() { SetHighlight(false); }

void UMenuButton::SetHighlight(bool bOn)
{
	if (Label) Label->SetColorAndOpacity(bOn ? HoverColor : NormalColor);

	const ESlateVisibility Vis = bOn ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Hidden;
	if (Arrow)     Arrow->SetVisibility(Vis);
	if (AccentBar) AccentBar->SetVisibility(Vis);
}