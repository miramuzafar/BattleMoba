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

			ABattleMobaPlayerState* PS = Cast <ABattleMobaPlayerState>(MobaPC->PlayerState);
			if (PS)
			{
				GState = Cast<ABattleMobaGameState>(UGameplayStatics::GetGameState(this));
				if (GState)
				{
					PS->Pi = Players.Num() - 1;
					
					//set mesh array into temp array
					Chars = CharSelections;

					//Random unique number for character mesh array
					if (Chars.IsValidIndex(0))
					{
						CharIndex = FMath::RandRange(0, Chars.Num() - 1);
						//PS->CharMesh = Chars[CharIndex];
						GEngine->AddOnScreenDebugMessage(-1, 10.0f, FColor::Yellow, FString::Printf(TEXT("Player Index : %d"), Players.Num() - 1));
					}
					if ((PS->Pi) < 4)
					{
						GState->TeamA.Add(PS->GetPlayerName());
						SpawnBasedOnTeam("Radiant", Chars[CharIndex]);
					}
					else
					{
						GState->TeamB.Add(PS->GetPlayerName());
						SpawnBasedOnTeam("Dire", Chars[CharIndex]);
					}
					Chars.RemoveAtSwap(CharIndex);
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

void ABattleMobaGameMode::SpawnBasedOnTeam/*_Implementation*/(FName TeamName, USkeletalMesh* CharMesh)
{
	ABattleMobaPlayerState* PS = Cast <ABattleMobaPlayerState>(newPlayer->PlayerState);
	if (PS)
	{
		if (HasAuthority())
		{
			PS->TeamName = TeamName;
			PS->CharMesh = CharMesh;

			AActor* PStart = FindPlayerStart(newPlayer, FString::FromInt(PS->Pi));

			//destroys existing pawn before spawning a new one
			if (newPlayer->GetPawn() != nullptr)
			{
				newPlayer->GetPawn()->Destroy();
			}

			ABattleMobaCharacter* pawn = GetWorld()->SpawnActorDeferred<ABattleMobaCharacter>(SpawnedActor, PStart->GetActorTransform(), nullptr, nullptr, ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn);
			if (pawn)
			{
				pawn->PlayerName = PS->GetPlayerName();
				pawn->TeamName = PS->TeamName;

				pawn->CharMesh = PS->CharMesh;
				PS->SpawnTransform = PStart->GetActorTransform();
				UGameplayStatics::FinishSpawningActor(pawn, FTransform(PStart->GetActorRotation(), PStart->GetActorLocation()));

				newPlayer->Possess(pawn);
				newPlayer->ClientSetRotation(PStart->GetActorRotation());

				newPlayer->bShowMouseCursor = false;
				newPlayer->SetInputMode(FInputModeGameOnly());
				//newPlayer->DisableInput(newPlayer);
			}
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

void ABattleMobaGameMode::StartRespawnTimer(ABattleMobaPlayerState* ps)
{
	FTimerDelegate FunctionsName;
	
	//FunctionsName = FTimerDelegate::CreateUObject(this, &ATodakBattleArenaCharacter::UpdateHealthStatusBar, EBarType::PrimaryProgressBar);
	FunctionsName = FTimerDelegate::CreateUObject(this, &ABattleMobaGameMode::RespawnTimerCount, &ps->RespawnHandle, ps);
			
	UE_LOG(LogTemp, Warning, TEXT("RespawnTimer started!"));
	GetWorld()->GetTimerManager().SetTimer(ps->RespawnHandle, FunctionsName, 1.0f, true);
}

void ABattleMobaGameMode::RespawnTimerCount(FTimerHandle* RespawnHandle, ABattleMobaPlayerState* ps)
{
	if (ps->RespawnTimeCounter >= 1)
	{
		ps->RespawnTimeCounter -= 1;
		ps->DisplayRespawnTime();
	}
	else
	{
		ps->RespawnTimeCounter = 30;
		ps->DisplayRespawnTime();
		this->GetWorldTimerManager().ClearTimer(*RespawnHandle);
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
			//destroys existing pawn before spawning a new one
			if (playerController->GetPawn() != nullptr)
			{
				playerController->GetPawn()->Destroy();
			}
			ABattleMobaPlayerState* PS = Cast<ABattleMobaPlayerState>(playerController->PlayerState);
			{
				//Spawn actor
				if (SpawnedActor)
				{
					//Spawn actor from SpawnedActor subclass
					ABattleMobaCharacter* pawn = GetWorld()->SpawnActorDeferred<ABattleMobaCharacter>(SpawnedActor, SpawnTransform, nullptr, nullptr, ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn);
					if (pawn)
					{
						//Assign team and player name before finish spawning
						pawn->PlayerName = playerController->PlayerState->GetPlayerName();
						pawn->TeamName = PS->TeamName;
						pawn->CharMesh = PS->CharMesh;

						UGameplayStatics::FinishSpawningActor(pawn, FTransform(SpawnTransform.Rotator(), SpawnTransform.GetLocation()));

						//possess and set new rotation for newly spawned pawn
						playerController->Possess(pawn);
						playerController->ClientSetRotation(pawn->GetActorRotation());
						playerController->bShowMouseCursor = false;
						playerController->SetInputMode(FInputModeGameOnly());
					}
				}
			}
		}
	}
}
