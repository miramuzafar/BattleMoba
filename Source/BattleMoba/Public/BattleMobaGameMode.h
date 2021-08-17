// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "Net/UnrealNetwork.h"
#include "BattleMobaGameMode.generated.h"

class ABattleMobaCharacter;

UCLASS(minimalapi)
class ABattleMobaGameMode : public AGameModeBase
{
	GENERATED_BODY()

protected:

	/*Blueprint Reference of ThirdPersonCharacter class*/
	UPROPERTY(EditDefaultsOnly, Category = "ActorSpawning")
	TSubclassOf<ABattleMobaCharacter> SpawnedActor;

public:
	ABattleMobaGameMode();

	virtual AActor* ChoosePlayerStart_Implementation(AController* Player) override;

	UFUNCTION(Reliable, Server, WithValidation, Category = "Respawn")
	void RespawnRequested(APlayerController* playerController, FTransform SpawnTransform);
};



