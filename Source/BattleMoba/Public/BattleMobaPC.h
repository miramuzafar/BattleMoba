// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "BattleMobaPC.generated.h"

/**
 * 
 */
UCLASS()
class BATTLEMOBA_API ABattleMobaPC : public APlayerController
{
	GENERATED_BODY()

public:

	//RequestRespawn
	UFUNCTION(Reliable, Server, WithValidation, Category = "Respawn")
	void RespawnPawn(FTransform SpawnTransform);
	
	/*UFUNCTION(Reliable, Server, WithValidation, Category = "Spectating")
	void SpectatingAlly();*/
};
