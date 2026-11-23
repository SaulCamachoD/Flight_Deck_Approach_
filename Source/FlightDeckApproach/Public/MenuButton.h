#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "MenuButton.generated.h"

class UButton;
class UTextBlock;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnMenuButtonClicked);

// Botón del menú: al pasar el mouse se iluminan el texto, la barra izquierda y la flecha
UCLASS()
class FLIGHTDECKAPPROACH_API UMenuButton : public UUserWidget
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintAssignable, Category="Menu") FOnMenuButtonClicked OnClicked;

protected:
	virtual void NativePreConstruct() override;
	virtual void NativeConstruct() override;

	UPROPERTY(meta=(BindWidget)) UButton* Button;
	UPROPERTY(meta=(BindWidget)) UTextBlock* Label;
	UPROPERTY(meta=(BindWidgetOptional)) UWidget* Arrow;
	UPROPERTY(meta=(BindWidgetOptional)) UWidget* AccentBar;

	// Se edita en cada instancia dentro de WBP_MainMenu
	UPROPERTY(EditAnywhere, Category="Menu") FText Text = INVTEXT("BUTTON");
	UPROPERTY(EditAnywhere, Category="Menu") FLinearColor NormalColor = FLinearColor::FromSRGBColor(FColor(0xE6, 0xEE, 0xF2));
	UPROPERTY(EditAnywhere, Category="Menu") FLinearColor HoverColor  = FLinearColor::FromSRGBColor(FColor(0x7F, 0xE9, 0xFF));

private:
	UFUNCTION() void HandleClicked();
	UFUNCTION() void HandleHovered();
	UFUNCTION() void HandleUnhovered();
	void SetHighlight(bool bOn);
};