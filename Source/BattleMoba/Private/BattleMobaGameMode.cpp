// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#include "BattleMobaGameMode.h"
#include "BattleMobaCharacter.h"
#include "EngineUtils.h"
#include "UObject/ConstructorHelpers.h"
#include "GameFramework/PlayerStart.h"

ABattleMobaGameMode::ABattleMobaGameMode()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/ThirdPersonCPP/Blueprints/ThirdPersonCharacter"));
	if (PlayerPawnBPClass.Class != NULL)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
	}
}

AActor* ABattleMobaGameMode::ChoosePlayerStart_Implementation(AController* Player)
{
	UE_LOG(LogTemp, Warning, TEXT("Player"));
	for (TActorIterator<APlayerStart> It(GetWorld()); It; ++It)
	{
		APlayerStart* currentPlayerStart = *It;
		if (currentPlayerStart->PlayerStartTag != "Taken")
		{
			currentPlayerStart->PlayerStartTag = "Taken";
			return currentPlayerStart;
		}
	}
	return nullptr;
}
