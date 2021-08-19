// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#include "BattleMobaGameMode.h"
#include "Engine.h"
#include "EngineUtils.h"
#include "UObject/ConstructorHelpers.h"
#include "GameFramework/PlayerStart.h"
#include "Net/UnrealNetwork.h"
#include "TimerManager.h"

//BattleMoba
#include "BattleMobaCharacter.h"
#include "BattleMobaPlayerState.h"

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

bool ABattleMobaGameMode::RespawnRequested_Validate(APlayerController* playerController, FTransform SpawnTransform)
{
	return true;
}

void ABattleMobaGameMode::RespawnRequested_Implementation(APlayerController* playerController, FTransform SpawnTransform)
{
	if (playerController != nullptr)
	{
		if (HasAuthority())
		{
			//Spawn actor
			if (SpawnedActor)
			{
				//Spawn actor from SpawnedActor subclass
				ABattleMobaCharacter* pawn = GetWorld()->SpawnActorDeferred<ABattleMobaCharacter>(SpawnedActor, SpawnTransform);
				if (pawn)
				{
					ABattleMobaPlayerState* PS = Cast<ABattleMobaPlayerState>(playerController->PlayerState);
					if (PS)
					{
						//Assign team and player name before finish spawning
						pawn->PlayerName = playerController->PlayerState->GetPlayerName();
						pawn->TeamName = PS->TeamName;
					}
					UGameplayStatics::FinishSpawningActor(pawn, FTransform(SpawnTransform.Rotator(), SpawnTransform.GetLocation()));
				}

				FTimerHandle handle;
				FTimerDelegate TimerDelegate;

				//Possess a pawn
				TimerDelegate.BindLambda([playerController, pawn]()
				{
					UE_LOG(LogTemp, Warning, TEXT("DELAY BEFORE POSSESSING A PAWN"));
					playerController->Possess(pawn);
				});
				this->GetWorldTimerManager().SetTimer(handle, TimerDelegate, 0.2f, false);
			}
		}
	}
}
