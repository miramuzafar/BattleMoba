// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "BattleMobaPlayerState.generated.h"

/**
 * 
 */
UCLASS()
class BATTLEMOBA_API ABattleMobaPlayerState : public APlayerState
{
	GENERATED_BODY()
	
public:
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Status")
	FName TeamName;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Status")
	int32 Pi = 0;
};
