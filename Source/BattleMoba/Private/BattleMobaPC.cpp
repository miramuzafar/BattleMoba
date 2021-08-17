// Fill out your copyright notice in the Description page of Project Settings.


#include "BattleMobaPC.h"
#include "Engine.h"
#include "Kismet/GameplayStatics.h"
#include "BattleMobaGameMode.h"

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

		//Possess a pawn
		TimerDelegate.BindLambda([this, thisGameMode, SpawnTransform]()
		{
			thisGameMode->RespawnRequested(this, SpawnTransform);
		});
		this->GetWorldTimerManager().SetTimer(handle, TimerDelegate, 3.0f, false);
	}
}
