// Fill out your copyright notice in the Description page of Project Settings.


#include "ILSTransmitter.h"
#include "Components/ArrowComponent.h"

AILSTransmitter::AILSTransmitter()
{
	PrimaryActorTick.bCanEverTick = true;
		
	DirectionArrow = CreateDefaultSubobject<UArrowComponent>(TEXT("DirectionArrow"));
	SetRootComponent(DirectionArrow);
	DirectionArrow->ArrowSize = 10.f;
	DirectionArrow->ArrowLength = 200.f;
	DirectionArrow->SetHiddenInGame(true);

}
