// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "BattleMobaGameState.generated.h"

/**
 * 
 */
UCLASS()
class BATTLEMOBA_API ABattleMobaGameState : public AGameStateBase
{
	GENERATED_BODY()

	//Replicated Network setup
	void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	
public:

	//--------------------Teams setup-------------------------//
	UPROPERTY(VisibleAnywhere, Replicated, BlueprintReadWrite)
		TArray <FString> TeamA;

	UPROPERTY(VisibleAnywhere, Replicated, BlueprintReadWrite)
		TArray <FString> TeamB;

	//KDA setup
	UPROPERTY(VisibleAnywhere, Replicated, BlueprintReadWrite)
		int TeamKillA = 0;

	UPROPERTY(VisibleAnywhere, Replicated, BlueprintReadWrite)
		int TeamKillB = 0;

	UPROPERTY(VisibleAnywhere, Replicated, BlueprintReadWrite)
		float Timer = 0.0f;

	//------------------Clock--------------------------//
	UPROPERTY(BlueprintReadWrite, Replicated, Category = "Clock")
		float CurrentTime = 0.0f;

	UPROPERTY(BlueprintReadWrite, Replicated, Category = "Clock")
		FString Winner;
};
