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

	// Punto de anclaje de marcas y números:
	//  vertical   -> borde izquierdo o derecho, a media altura
	//  horizontal -> borde inferior, en el centro
	const float Edge = bLabelsOnLeft ? 1.f : 0.f;
	const FVector2D Anchor = bHorizontal ? FVector2D(0.5f, 1.f) : FVector2D(Edge, 0.5f);

	// Hacia dónde se separan los números de la línea
	const FVector2D LabelOffset = bHorizontal
		? FVector2D(0.f, -20.f)
		: FVector2D(bLabelsOnLeft ? -20.f : 20.f, 0.f);

	// --- Línea de la escala ---
	UImage* Line = WidgetTree->ConstructWidget<UImage>(UImage::StaticClass());
	Line->SetColorAndOpacity(TapeColor);
	UCanvasPanelSlot* LineSlot = TapeCanvas->AddChildToCanvas(Line);
	if (bHorizontal)
	{
		LineSlot->SetAnchors(FAnchors(0.f, 1.f, 1.f, 1.f));   // todo el ancho, abajo
		LineSlot->SetAlignment(FVector2D(0.f, 1.f));
		LineSlot->SetOffsets(FMargin(0.f, 0.f, 0.f, 2.f));   // alto 2 px
	}
	else
	{
		LineSlot->SetAnchors(FAnchors(Edge, 0.f, Edge, 1.f));   // todo el alto, en un lado
		LineSlot->SetAlignment(FVector2D(Edge, 0.f));
		LineSlot->SetOffsets(FMargin(0.f, 0.f, 2.f, 0.f));   // ancho 2 px
	}

	// --- Marcas ---
	const int32 TickCount = LabelCount * MinorPerMajor;
	for (int32 i = 0; i < TickCount; ++i)
	{
		UImage* Tick = WidgetTree->ConstructWidget<UImage>(UImage::StaticClass());
		Tick->SetColorAndOpacity(TapeColor);
		Tick->SetRenderTransformPivot(Anchor);   // al encoger, se queda pegada a la línea
		UCanvasPanelSlot* S = TapeCanvas->AddChildToCanvas(Tick);
		S->SetAnchors(FAnchors(Anchor.X, Anchor.Y));
		S->SetAlignment(Anchor);
		S->SetPosition(FVector2D::ZeroVector);
		S->SetSize(bHorizontal ? FVector2D(2.f, 14.f) : FVector2D(14.f, 2.f));
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
		S->SetAnchors(FAnchors(Anchor.X, Anchor.Y));
		S->SetAlignment(Anchor);
		S->SetPosition(LabelOffset);
		S->SetAutoSize(true);
		Labels.Add(Label);
	}

	SetValue(Value);
}

FText UTapeWidget::FormatLabel(float LabelValue) const
{
	if (bWrap360)
	{
		int32 Deg = FMath::RoundToInt(LabelValue) % 360;
		if (Deg < 0) Deg += 360;
		const int32 Num = (Deg == 0) ? 36 : Deg / 10;   // 0° -> "36", 30° -> "03"
		return FText::FromString(FString::Printf(TEXT("%02d"), Num));
	}
	return FText::AsNumber(FMath::RoundToInt(LabelValue));
}

void UTapeWidget::SetValue(float NewValue)
{
	Value = NewValue;
	if (Labels.Num() == 0) return;

	const float PxPerUnit = PxPerMajor / MajorStep;

	// Coloca un elemento según su valor relativo al centro de la cinta.
	// Vertical: valores altos ARRIBA (Y negativa). Horizontal: valores altos a la DERECHA.
	auto Place = [this, PxPerUnit](UWidget* W, float ItemValue)
	{
		const float Px = (ItemValue - Value) * PxPerUnit;
		W->SetRenderTranslation(bHorizontal ? FVector2D(Px, 0.f) : FVector2D(0.f, -Px));
	};
	// En rumbo no hay negativos: -10° es 350°
	auto Shown = [this](float ItemValue)
	{
		return (bWrap360 || bShowNegative || ItemValue >= 0.f)
			? ESlateVisibility::HitTestInvisible : ESlateVisibility::Hidden;
	};

	// --- Números ---
	const float BaseMajor = FMath::FloorToFloat(Value / MajorStep) * MajorStep;
	const int32 HalfLabels = Labels.Num() / 2;
	for (int32 i = 0; i < Labels.Num(); ++i)
	{
		const float LabelValue = BaseMajor + (i - HalfLabels) * MajorStep;
		Place(Labels[i], LabelValue);
		Labels[i]->SetText(FormatLabel(LabelValue));
		Labels[i]->SetVisibility(Shown(LabelValue));
	}

	// --- Marcas: largas en cada número, cortas entre medias ---
	const float MinorStep = MajorStep / MinorPerMajor;
	const float BaseMinor = FMath::FloorToFloat(Value / MinorStep) * MinorStep;
	const int32 HalfTicks = Ticks.Num() / 2;
	for (int32 i = 0; i < Ticks.Num(); ++i)
	{
		const float TickValue = BaseMinor + (i - HalfTicks) * MinorStep;
		const bool bMajor = FMath::RoundToInt(TickValue / MinorStep) % MinorPerMajor == 0;
		const float S = bMajor ? 1.f : 0.5f;
		Place(Ticks[i], TickValue);
		Ticks[i]->SetRenderScale(bHorizontal ? FVector2D(1.f, S) : FVector2D(S, 1.f));
		Ticks[i]->SetVisibility(Shown(TickValue));
	}
}


