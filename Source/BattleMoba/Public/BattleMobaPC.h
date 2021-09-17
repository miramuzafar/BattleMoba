// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "Net/UnrealNetwork.h"
#include "BattleMobaPC.generated.h"

/**
 * 
 */
UCLASS()
class BATTLEMOBA_API ABattleMobaPC : public APlayerController
{
	GENERATED_BODY()

	ABattleMobaPC();

	//Replicated Network setup
	void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

public:

	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, Replicated, BlueprintReadWrite, Category = "ID")
		int32 pi;

	//RequestRespawn
	UFUNCTION(Reliable, Server, WithValidation, Category = "Respawn")
	void RespawnPawn(FTransform SpawnTransform);
};
