// Fill out your copyright notice in the Description page of Project Settings.


#include "BattleMobaPC.h"
#include "Engine.h"
#include "Kismet/GameplayStatics.h"

//BattleMoba
#include "BattleMobaGameMode.h"
#include "BattleMobaPlayerState.h"

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
			this->GetPawn()->SetLifeSpan(5.0f);
		}

		TArray<APlayerState*> PStates = UGameplayStatics::GetGameState(this)->PlayerArray;

		ABattleMobaPlayerState* PS = Cast<ABattleMobaPlayerState>(this->PlayerState);

		for (auto& ps : PStates)
		{
			if (ps != this->PlayerState)
			{
				ABattleMobaPlayerState* pstate = Cast<ABattleMobaPlayerState>(ps);
				if (PS->TeamName == pstate->TeamName)
				{
					APlayerController* pc = Cast<APlayerController>(UGameplayStatics::GetPlayerController(this, pstate->Pi));
					this->SetViewTargetWithBlend(pc, 2.0f);
					break;
				}
			}
		}

		FTimerHandle handle;
		FTimerDelegate TimerDelegate;

		//Possess a pawn
		TimerDelegate.BindLambda([this, thisGameMode, SpawnTransform]()
		{
			thisGameMode->RespawnRequested(this, SpawnTransform);
		});
		this->GetWorldTimerManager().SetTimer(handle, TimerDelegate, 15.0f, false);
	}
}
