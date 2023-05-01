// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "MyPlayerController.generated.h"

class UStatsWidget;

UCLASS()
class CYBERLIFE_API AMyPlayerController : public APlayerController
{
	GENERATED_BODY()

private:
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UUserWidget> PointScreen;
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UUserWidget> Stats;

	UPROPERTY(BlueprintReadWrite, meta=(AllowPrivateAccess))
	UStatsWidget* StatsWidget;
	
	virtual void BeginPlay() override;
};
