// Fill out your copyright notice in the Description page of Project Settings.


#include "TapeWidget.h"
#include "Blueprint/WidgetTree.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/TextBlock.h"
#include "Components/Image.h"

void UTapeWidget::NativeConstruct()
{
	Super::NativeConstruct();
	if (!TapeCanvas || Labels.Num() > 0) return;   // ya construida

	// Lado donde va la línea: 1 = borde derecho, 0 = borde izquierdo
	const float Edge = bLabelsOnLeft ? 1.f : 0.f;
	const float Dir  = bLabelsOnLeft ? -1.f : 1.f;   // hacia dónde crecen los números

	// --- Línea vertical de la escala, de arriba a abajo ---
	UImage* Line = WidgetTree->ConstructWidget<UImage>(UImage::StaticClass());
	Line->SetColorAndOpacity(TapeColor);
	UCanvasPanelSlot* LineSlot = TapeCanvas->AddChildToCanvas(Line);
	LineSlot->SetAnchors(FAnchors(Edge, 0.f, Edge, 1.f));
	LineSlot->SetAlignment(FVector2D(Edge, 0.f));
	LineSlot->SetOffsets(FMargin(0.f, 0.f, 2.f, 0.f));   // ancho 2 px, alto completo

	// --- Marcas ---
	const int32 TickCount = LabelCount * MinorPerMajor;
	for (int32 i = 0; i < TickCount; ++i)
	{
		UImage* Tick = WidgetTree->ConstructWidget<UImage>(UImage::StaticClass());
		Tick->SetColorAndOpacity(TapeColor);
		Tick->SetRenderTransformPivot(FVector2D(Edge, 0.5f));   // al encoger, se queda pegada a la línea
		UCanvasPanelSlot* S = TapeCanvas->AddChildToCanvas(Tick);
		S->SetAnchors(FAnchors(Edge, 0.5f));
		S->SetAlignment(FVector2D(Edge, 0.5f));
		S->SetPosition(FVector2D::ZeroVector);
		S->SetSize(FVector2D(14.f, 2.f));
		Ticks.Add(Tick);
	}

	// --- Números ---
	for (int32 i = 0; i < LabelCount; ++i)
	{
		UTextBlock* Label = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass());
		Label->SetColorAndOpacity(FSlateColor(TapeColor));
		FSlateFontInfo Font = Label->GetFont();
		Font.Size = FontSize;
		Label->SetFont(Font);
		UCanvasPanelSlot* S = TapeCanvas->AddChildToCanvas(Label);
		S->SetAnchors(FAnchors(Edge, 0.5f));
		S->SetAlignment(FVector2D(Edge, 0.5f));
		S->SetPosition(FVector2D(Dir * 20.f, 0.f));   // 20 px separados de la línea
		S->SetAutoSize(true);
		Labels.Add(Label);
	}

	SetValue(Value);
}


void UTapeWidget::SetValue(float NewValue)
{
	Value = NewValue;
	if (Labels.Num() == 0) return;

	const float PxPerUnit = PxPerMajor / MajorStep;

	// --- Números: el múltiplo de MajorStep justo por debajo del valor queda en el medio ---
	const float BaseMajor = FMath::FloorToFloat(Value / MajorStep) * MajorStep;
	const int32 HalfLabels = Labels.Num() / 2;
	for (int32 i = 0; i < Labels.Num(); ++i)
	{
		const float LabelValue = BaseMajor + (i - HalfLabels) * MajorStep;
		// Valores más altos van ARRIBA, y en pantalla arriba es Y negativa
		const float Y = -(LabelValue - Value) * PxPerUnit;
		Labels[i]->SetRenderTranslation(FVector2D(0.f, Y));
		Labels[i]->SetText(FText::AsNumber(FMath::RoundToInt(LabelValue)));
		Labels[i]->SetVisibility(bShowNegative || LabelValue >= 0.f
			? ESlateVisibility::HitTestInvisible : ESlateVisibility::Hidden);
	}

	// --- Marcas: largas en cada número, cortas entre medias ---
	const float MinorStep = MajorStep / MinorPerMajor;
	const float BaseMinor = FMath::FloorToFloat(Value / MinorStep) * MinorStep;
	const int32 HalfTicks = Ticks.Num() / 2;
	for (int32 i = 0; i < Ticks.Num(); ++i)
	{
		const float TickValue = BaseMinor + (i - HalfTicks) * MinorStep;
		const bool bMajor = FMath::RoundToInt(TickValue / MinorStep) % MinorPerMajor == 0;
		Ticks[i]->SetRenderTranslation(FVector2D(0.f, -(TickValue - Value) * PxPerUnit));
		Ticks[i]->SetRenderScale(FVector2D(bMajor ? 1.f : 0.5f, 1.f));
		Ticks[i]->SetVisibility(bShowNegative || TickValue >= 0.f
			? ESlateVisibility::HitTestInvisible : ESlateVisibility::Hidden);
	}
	
}
