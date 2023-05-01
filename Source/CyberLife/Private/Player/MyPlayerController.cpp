// Fill out your copyright notice in the Description page of Project Settings.


#include "Player/MyPlayerController.h"

#include "Blueprint/UserWidget.h"
#include "UI/StatsWidget.h"

void AMyPlayerController::BeginPlay()
{
	Super::BeginPlay();

	//CreateWidget(this, PointScreen)->AddToViewport();
	//StatsWidget = Cast<UStatsWidget>(CreateWidget(this, Stats));
	//StatsWidget->AddToViewport();
}
