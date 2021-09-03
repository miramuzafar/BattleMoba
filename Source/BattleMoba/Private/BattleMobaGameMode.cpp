// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#include "BattleMobaGameMode.h"
#include "Engine.h"
#include "EngineUtils.h"
#include "UObject/ConstructorHelpers.h"
#include "GameFramework/PlayerStart.h"
#include "Net/UnrealNetwork.h"
#include "Math/UnrealMathUtility.h"
#include "TimerManager.h"
#include "Kismet/KismetArrayLibrary.h"

//BattleMoba
#include "BattleMobaCharacter.h"
#include "BattleMobaPlayerState.h"
#include "BattleMobaGameState.h"
#include "BattleMobaPC.h"
#include "InputLibrary.h"
#include "Net/UnrealNetwork.h"

ABattleMobaGameMode::ABattleMobaGameMode()
{
	//// set default pawn class to our Blueprinted character
	//static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/ThirdPersonCPP/Blueprints/ThirdPersonCharacter"));
	//if (PlayerPawnBPClass.Class != NULL)
	//{
	//	DefaultPawnClass = PlayerPawnBPClass.Class;
	//}
}

void ABattleMobaGameMode::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ABattleMobaGameMode, Players);
	//DOREPLIFETIME(ABattleMobaGameMode, CharIndex);
	//DOREPLIFETIME(ABattleMobaGameMode, CharSelections);
}

void ABattleMobaGameMode::BeginPlay()
{
	Super::BeginPlay();

	Chars = CharSelections;
	
	//sETTING UP GAME STATE
	GState = Cast<ABattleMobaGameState>(UGameplayStatics::GetGameState(this));

	if (HasAuthority() && (UGameplayStatics::GetCurrentLevelName(this) == MapName))
	{
		FTimerHandle handle;
		FTimerDelegate TimerDelegate;

		//Possess a pawn
		TimerDelegate.BindLambda([this]()
		{
			GEngine->AddOnScreenDebugMessage(-1, 10.0f, FColor::Magenta, FString::Printf(TEXT("Start Timer")));
			//Get Initial time
			InitialTimer = UInputLibrary::GetCurrentTimeInMinute()*60.0f;

			//Start game time
			//GEngine->AddOnScreenDebugMessage(-1, 10.0f, FColor::Yellow, FString::Printf(TEXT("initial time : %d"), InitialTimer));
			GetWorldTimerManager().SetTimer(ClockTimer, this, &ABattleMobaGameMode::StartClock, 1.0f, true);
		});
		this->GetWorldTimerManager().SetTimer(handle, TimerDelegate, 1.0f, false);
	}
}

void ABattleMobaGameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);

	newPlayer = NewPlayer;
	if (HasAuthority())
	{
		GEngine->AddOnScreenDebugMessage(-1, 10.0f, FColor::Blue, FString::Printf(TEXT("Player Added : %s"), *newPlayer->GetFName().ToString()));

		//newPlayer = NewPlayer;
		ABattleMobaPC* MobaPC = Cast<ABattleMobaPC>(newPlayer);

		if (MobaPC)
		{
			Players.Add(MobaPC);

			if (newPlayer->GetPawn() != nullptr)
			{
				newPlayer->Destroy(newPlayer->GetPawn());
			}

			ABattleMobaPlayerState* PS = Cast <ABattleMobaPlayerState>(MobaPC->PlayerState);
			if (PS)
			{
				GState = Cast<ABattleMobaGameState>(UGameplayStatics::GetGameState(this));
				if (GState)
				{
					PS->Pi = Players.Num() - 1;
					//PS->SetPlayerIndex(PS->Pi);

					if (Chars.IsValidIndex(0))
					{
						//Random unique number for character mesh array
						CharIndex = FMath::RandRange(0, Chars.Num() - 1);
						PS->CharMesh = Chars[CharIndex];
						Chars.RemoveAtSwap(CharIndex);
						GEngine->AddOnScreenDebugMessage(-1, 10.0f, FColor::Yellow, FString::Printf(TEXT("Player Index : %d"), Players.Num() - 1));
					}
					if ((PS->Pi) < 3)
					{
						GState->TeamA.Add(PS->GetPlayerName());
						SpawnBasedOnTeam("Radiant");
					}
					else
					{
						GState->TeamB.Add(PS->GetPlayerName());
						SpawnBasedOnTeam("Dire");
					}
				}
			}
		}
	}
}

void ABattleMobaGameMode::StartClock()
{
	if (GState != nullptr)
	{
		if (CurrentTime < EndTime)
		{
			//Get current timer value
			CurrentTime = (UInputLibrary::GetCurrentTimeInMinute()*60.0f) - InitialTimer;
			GState->CurrentTime = CurrentTime;
			//GEngine->AddOnScreenDebugMessage(-1, 10.0f, FColor::Yellow, FString::Printf(TEXT("Current time : %f"), GState->CurrentTime));
		}
		else
		{
			//Stops the timer and check for winners
			GetWorldTimerManager().ClearTimer(ClockTimer);
			if (GState->TeamKillA > GState->TeamKillB)
			{
				GState->Winner = "Radiant Wins";
			}
			else if (GState->TeamKillB > GState->TeamKillA)
			{
				GState->Winner = "Dire Wins";
			}
			else
				GState->Winner = "Draw";
		}
	}
	
}

//bool ABattleMobaGameMode::SpawnBasedOnTeam_Validate(FName TeamName)
//{
//	return true;
//}

void ABattleMobaGameMode::SpawnBasedOnTeam/*_Implementation*/(FName TeamName)
{
	ABattleMobaPlayerState* PS = Cast <ABattleMobaPlayerState>(newPlayer->PlayerState);
	if (PS)
	{
		PS->TeamName = TeamName;

		/*for (TActorIterator<APlayerStart> It(GetWorld()); It; ++It)
		{
			APlayerStart* currentPlayerStart = *It;
			if (currentPlayerStart->PlayerStartTag == FName(*FString::FromInt(PS->Pi)))
			{
				PStart = currentPlayerStart;
				break;
			}
		}*/
		AActor* PStart = FindPlayerStart(newPlayer, FString::FromInt(PS->Pi));

		ABattleMobaCharacter* pawn = GetWorld()->SpawnActorDeferred<ABattleMobaCharacter>(SpawnedActor, PStart->GetActorTransform());
		if (pawn)
		{
			pawn->PlayerName = PS->GetPlayerName();
			pawn->TeamName = PS->TeamName;
			pawn->CharMesh = PS->CharMesh;
			PS->SpawnTransform = PStart->GetActorTransform();
			UGameplayStatics::FinishSpawningActor(pawn, FTransform(PStart->GetActorRotation(), PStart->GetActorLocation()));

			newPlayer->Possess(pawn);
			newPlayer->ClientSetRotation(PStart->GetActorRotation());
		}

	}
}

//AActor* ABattleMobaGameMode::ChoosePlayerStart_Implementation(AController* Player)
//{
//	UE_LOG(LogTemp, Warning, TEXT("Player"));
//	for (TActorIterator<APlayerStart> It(GetWorld()); It; ++It)
//	{
//		APlayerStart* currentPlayerStart = *It;
//		if (currentPlayerStart->PlayerStartTag != "Taken")
//		{
//			currentPlayerStart->PlayerStartTag = "Taken";
//			return currentPlayerStart;
//		}
//	}
//	return nullptr;
//}

void ABattleMobaGameMode::PlayerKilled(ABattleMobaPlayerState* victim, ABattleMobaPlayerState* killer, TArray<ABattleMobaPlayerState*> assist)
{
	if (assist.IsValidIndex(0))
	{
		for (int32 i = 0; i < assist.Num()-1; i++)
		{
			if (i < assist.Num() - 1)
			{
				assist[i]->Assist += 1;
			}
		}
	}
	if (killer != victim)
	{
		victim->Death += 1;
		killer->Kill += 1;

		if (killer->TeamName == "Radiant")
		{
			GState->TeamKillA += 1;
		}
		else if(killer->TeamName == "Dire")
		{
			GState->TeamKillB += 1;
		}
		/*if (GState->TeamA.Contains(killer->GetPlayerName()))
		{
			GState->TeamKillA += 1;
		}
		else
			GState->TeamKillB += 1;*/
	}
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
			ABattleMobaPlayerState* PS = Cast<ABattleMobaPlayerState>(playerController->PlayerState);
			{
				//Spawn actor
				if (SpawnedActor)
				{
					//Spawn actor from SpawnedActor subclass
					ABattleMobaCharacter* pawn = GetWorld()->SpawnActorDeferred<ABattleMobaCharacter>(SpawnedActor, SpawnTransform);
					if (pawn)
					{
						//Assign team and player name before finish spawning
						pawn->PlayerName = playerController->PlayerState->GetPlayerName();
						pawn->TeamName = PS->TeamName;
						pawn->CharMesh = PS->CharMesh;

						UGameplayStatics::FinishSpawningActor(pawn, FTransform(SpawnTransform.Rotator(), SpawnTransform.GetLocation()));
					}

					FTimerHandle handle;
					FTimerDelegate TimerDelegate;

					//Possess a pawn
					TimerDelegate.BindLambda([playerController, pawn]()
					{
						UE_LOG(LogTemp, Warning, TEXT("DELAY BEFORE POSSESSING A PAWN"));
						playerController->Possess(pawn);
						playerController->ClientSetRotation(pawn->GetActorRotation());
					});
					this->GetWorldTimerManager().SetTimer(handle, TimerDelegate, 0.2f, false);
				}
			}
		}
	}
}
