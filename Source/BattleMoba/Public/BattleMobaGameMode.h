// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "Net/UnrealNetwork.h"
#include "BattleMobaGameMode.generated.h"

UCLASS(minimalapi)
class ABattleMobaGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	ABattleMobaGameMode();

	virtual AActor* ChoosePlayerStart_Implementation(AController* Player) override;
};



