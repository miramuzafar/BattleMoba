// Fill out your copyright notice in the Description page of Project Settings.


#include "BattleMobaGameState.h"
#include "Net/UnrealNetwork.h"

void ABattleMobaGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ABattleMobaGameState, TeamA);
	DOREPLIFETIME(ABattleMobaGameState, TeamB);
	DOREPLIFETIME(ABattleMobaGameState, TeamKillA);
	DOREPLIFETIME(ABattleMobaGameState, TeamKillB);
	DOREPLIFETIME(ABattleMobaGameState, Timer);
}
