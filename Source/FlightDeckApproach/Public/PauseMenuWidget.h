#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PauseMenuWidget.generated.h"

class UMenuButton;

// Overlay de pausa: RESUME / RETRY / MAIN MENU. Esc o P también reanudan
UCLASS()
class FLIGHTDECKAPPROACH_API UPauseMenuWidget : public UUserWidget
{
	GENERATED_BODY()

protected:
	virtual void NativeConstruct() override;
	virtual FReply NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent) override;

	UPROPERTY(meta=(BindWidget)) UMenuButton* ResumeButton;
	UPROPERTY(meta=(BindWidget)) UMenuButton* RetryButton;
	UPROPERTY(meta=(BindWidget)) UMenuButton* MainMenuButton;

private:
	UFUNCTION() void OnResumeClicked();
	UFUNCTION() void OnRetryClicked();
	UFUNCTION() void OnMainMenuClicked();

	class AFDAGameMode* GetFDAGameMode() const;
};