// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "TapeWidget.generated.h"

class UCanvasPanel;
class UTextBlock;
class UImage;

/**
 * 
 */
UCLASS()
class FLIGHTDECKAPPROACH_API UTapeWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	// Valor que queda en el centro de la cinta (kt, ft...)
	void SetValue(float NewValue);

protected:
	virtual void NativeConstruct() override;

	// Raíz del WBP_Tape: aquí se crean línea, marcas y números (Clip to Bounds)
	UPROPERTY(meta=(BindWidget)) UCanvasPanel* TapeCanvas;

	UPROPERTY(EditAnywhere, Category="Tape") float MajorStep = 20.f;       // cada cuánto hay número
	UPROPERTY(EditAnywhere, Category="Tape") int32 MinorPerMajor = 2;      // marcas por cada número
	UPROPERTY(EditAnywhere, Category="Tape") float PxPerMajor = 90.f;      // píxeles entre números
	UPROPERTY(EditAnywhere, Category="Tape") int32 LabelCount = 7;         // números reciclados
	UPROPERTY(EditAnywhere, Category="Tape") bool bLabelsOnLeft = false;   // línea a la derecha y números a la izquierda
	UPROPERTY(EditAnywhere, Category="Tape") bool bShowNegative = false;
	UPROPERTY(EditAnywhere, Category="Tape") int32 FontSize = 16;
	UPROPERTY(EditAnywhere, Category="Tape") FLinearColor TapeColor = FLinearColor(0.2f, 1.f, 0.43f);
	UPROPERTY(EditAnywhere, Category="Tape") bool bHorizontal = false;   // cinta de rumbo: se desliza de lado
	UPROPERTY(EditAnywhere, Category="Tape") bool bWrap360 = false;      // 359 -> 0, números estilo "03", "36"

private:
	UPROPERTY() TArray<TObjectPtr<UTextBlock>> Labels;
	UPROPERTY() TArray<TObjectPtr<UImage>> Ticks;
	float Value = 0.f;
	FText FormatLabel(float LabelValue) const;
	
};
