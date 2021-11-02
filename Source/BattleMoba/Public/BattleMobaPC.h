// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "Net/UnrealNetwork.h"
#include "BattleMobaPC.generated.h"

class ABattleMobaGameMode;
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

	UPROPERTY()
	ABattleMobaGameMode* GM;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Spectator")
		int32 currentPlayer;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Spectator")
	ABattleMobaPC* CurrSpectator;

	UPROPERTY(VisibleAnywhere, Replicated, BlueprintReadWrite, Category = "ID")
		int32 pi;

	//RequestRespawn
	UFUNCTION(Reliable, Server, WithValidation, Category = "Respawn")
	void RespawnPawn(FTransform SpawnTransform);

protected:
	
	//spectator pi
	UPROPERTY(VisibleAnywhere, Replicated, BlueprintReadWrite, Category = "SpectID")
		int32 SpectPI;

	UFUNCTION(BlueprintPure, Category = "Increment")
		static int32 CheckIndexValidity(int32 index, TArray<ABattleMobaPC*> PlayerList, EFormula SwitchMode);

	//RequestSpectator
	UFUNCTION(BlueprintCallable, Reliable, Server, WithValidation, Category = "Spectator")
		void SetupSpectator(EFormula SwitchMode);

	//SpectatorMode
	UFUNCTION(Reliable, Server, WithValidation, Category = "Spectator")
		void SpectateNextPlayer(const TArray<ABattleMobaPC*>& PlayerList, EFormula SwitchMode);

};
