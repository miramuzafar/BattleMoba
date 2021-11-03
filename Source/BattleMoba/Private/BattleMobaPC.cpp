// Fill out your copyright notice in the Description page of Project Settings.


#include "BattleMobaPC.h"
#include "Engine.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"
#include "Camera/PlayerCameraManager.h"

//BattleMoba
#include "InputLibrary.h"
#include "BattleMobaGameMode.h"
#include "BattleMobaPlayerState.h"
#include "BattleMobaGameState.h"

ABattleMobaPC::ABattleMobaPC()
{
	//SetInputMode(FInputModeUIOnly());
}

void ABattleMobaPC::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ABattleMobaPC, pi);
}

void ABattleMobaPC::BeginPlay()
{
	Super::BeginPlay();
}

int32 ABattleMobaPC::CheckIndexValidity(int32 index, TArray<ABattleMobaPC*> PlayerList, EFormula SwitchMode)
{
	if (SwitchMode == EFormula::Addition)
	{
		//select the index number forward
		int32 temp = index + 1;
		if (PlayerList.Num() == temp)
		{
			return 0;
		}
		else
			return temp;
	}
	else
	{
		//select the index number backwards
		int32 temp = index - 1;
		if (temp == -1)
		{
			return PlayerList.Num() - 1;
		}
		else
			return temp;
	}
	
	
}

bool ABattleMobaPC::SpectateNextPlayer_Validate(const TArray<ABattleMobaPC*>& PlayerList, EFormula SwitchMode)
{
	return true;
}

void ABattleMobaPC::SpectateNextPlayer_Implementation(const TArray<ABattleMobaPC*>& PlayerList, EFormula SwitchMode)
{

	bool x = false;
	int32 count = 0;

	//get playerstate
	ABattleMobaPlayerState* thisps = Cast<ABattleMobaPlayerState>(this->PlayerState);

	for (int32 i = 0; i < PlayerList.Num(); i++)
	{
		if (PlayerList[i]->GetPawn() != nullptr)
		{
			ABattleMobaPlayerState* ps = Cast<ABattleMobaPlayerState>(PlayerList[i]->PlayerState);
			if (ps->TeamName == thisps->TeamName)
			{
				count += 1;
				if (count > 0)
				{
					x = true;
					break;
				}
			}
		}
	}
	
	//Is eligible to spectate??
	if (x == true && count > 0)
	{
	loop:
		currentPlayer = CheckIndexValidity(currentPlayer, PlayerList, SwitchMode); //check if next spectated player is exist in an array

		if (PlayerList[currentPlayer]->GetPawn() != nullptr) //checks if player does have a pawn
		{
			if (PlayerList[currentPlayer]->GetPawn() != this->GetPawn()) //checks if spectated pawn is not current owning pawn
			{
				ABattleMobaPlayerState* ps = Cast<ABattleMobaPlayerState>(PlayerList[currentPlayer]->PlayerState);
				if (ps->TeamName == thisps->TeamName) //spectating only team pawn
				{
					this->SetViewTargetWithBlend(PlayerList[currentPlayer], 0.0f, EViewTargetBlendFunction::VTBlend_Linear, 0.0f, true);
					CurrSpectator = PlayerList[currentPlayer];//set new spectated player
					CurrSpectator->SpectPI = this->pi;
					return;
				}
				else
					goto loop;
			}
			else
				goto loop;
		}
		else
			goto loop;
	}
}

bool ABattleMobaPC::SetupSpectator_Validate(EFormula SwitchMode)
{
	return true;
}

void ABattleMobaPC::SetupSpectator_Implementation(EFormula SwitchMode)
{
	if (this->GetPawn() == nullptr) // make sure no owning pawn present before spectating
	{
		GM = Cast<ABattleMobaGameMode>(UGameplayStatics::GetGameMode(this));
		if (GM)
		{
			SpectateNextPlayer(GM->Players, SwitchMode);
		}
	}
}

bool ABattleMobaPC::RespawnPawn_Validate(FTransform SpawnTransform)
{
	return true;
}

void ABattleMobaPC::RespawnPawn_Implementation(FTransform SpawnTransform)
{
	ABattleMobaGameMode* thisGameMode = Cast<ABattleMobaGameMode>(UGameplayStatics::GetGameMode(this));
	if (thisGameMode)
	{
		//Destroy pawn before respawning
		if (this->GetPawn())
		{
			this->GetPawn()->Destroy();
		}

		FTimerHandle handle;
		FTimerDelegate TimerDelegate;

		//set view target
		TimerDelegate.BindLambda([this, thisGameMode]()
		{
			//Assigned initial spectator player before swapping to active pawn player to spectate
			currentPlayer = thisGameMode->Players.Find(this);
			CurrSpectator = thisGameMode->Players[currentPlayer];
			CurrSpectator->SpectPI = this->pi;

			this->SpectateNextPlayer(thisGameMode->Players, EFormula::Addition);

			//Setup spectator controller that currently spectating this player to switch to another player
			ABattleMobaPC* newPC = Cast<ABattleMobaPC>(UGameplayStatics::GetPlayerController(GetWorld(), CurrSpectator->SpectPI));
			if (newPC)
			{
				if (newPC->GetPawn() == nullptr)
				{
					newPC->currentPlayer = thisGameMode->Players.Find(newPC);
					newPC->CurrSpectator = thisGameMode->Players[currentPlayer];
					newPC->CurrSpectator->SpectPI = newPC->pi;

					newPC->SpectateNextPlayer(thisGameMode->Players, EFormula::Addition);
				}
			}

		});
		this->GetWorldTimerManager().SetTimer(handle, TimerDelegate, 0.02f, false);

		//get current controller playerstate
		ABattleMobaPlayerState* thisstate = Cast<ABattleMobaPlayerState>(this->PlayerState);

		//Delay before respawning a new pawn
		FTimerHandle handle1;
		FTimerDelegate TimerDelegate1;

		//Possess a pawn
		TimerDelegate1.BindLambda([this, thisGameMode, thisstate]()
		{
			PlayerCameraManager->BlendTimeToGo = 0.0f;
			thisGameMode->RespawnRequested(this, thisstate->SpawnTransform);
		});
		this->GetWorldTimerManager().SetTimer(handle1, TimerDelegate1, 27.0f, false);
	}
}