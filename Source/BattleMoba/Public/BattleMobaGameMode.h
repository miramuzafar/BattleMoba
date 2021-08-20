// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "Net/UnrealNetwork.h"
#include "BattleMobaGameMode.generated.h"

class ABattleMobaCharacter;
class ABattleMobaGameState;

UCLASS(minimalapi)
class ABattleMobaGameMode : public AGameModeBase
{
	GENERATED_BODY()

protected:

	UPROPERTY(BlueprintReadWrite, Category = "Game State")
		ABattleMobaGameState* GState;

	/*Blueprint Reference of ThirdPersonCharacter class*/
	UPROPERTY(EditDefaultsOnly, Category = "ActorSpawning")
	TSubclassOf<ABattleMobaCharacter> SpawnedActor;

	//************************Clock***********************//
	UPROPERTY(BlueprintReadWrite, Category = "Clock")
		float InitialTimer = 0.0f;

	UPROPERTY(BlueprintReadWrite, Category = "Clock")
		float CurrentTime = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Clock")
		float EndTime = 900.0f;

	

protected:

	virtual void BeginPlay() override;

	UFUNCTION()
		void StartClock();

public:

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ActorSpawning")
	TArray<class ABattleMobaPC*> Players;

	UPROPERTY()
		FTimerHandle ClockTimer;

public:
	ABattleMobaGameMode();

	virtual AActor* ChoosePlayerStart_Implementation(AController* Player) override;

	UFUNCTION(Reliable, Server, WithValidation, Category = "Respawn")
	void RespawnRequested(APlayerController* playerController, FTransform SpawnTransform);
};



