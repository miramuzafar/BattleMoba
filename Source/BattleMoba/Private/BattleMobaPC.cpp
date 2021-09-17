// Fill out your copyright notice in the Description page of Project Settings.


#include "BattleMobaPC.h"
#include "Engine.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"
#include "Camera/PlayerCameraManager.h"

//BattleMoba
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

		//get current controller playerstate
		ABattleMobaPlayerState* thisstate = Cast<ABattleMobaPlayerState>(this->PlayerState);

		ABattleMobaGameState* gs = Cast<ABattleMobaGameState>(UGameplayStatics::GetGameState(this));
		if (gs)
		{
			TArray<APlayerState*> PStates = gs->PlayerArray;
			for (auto& ps : PStates)
			{
				if (this->PlayerState != ps)
				{
					ABattleMobaPlayerState* pstate = Cast<ABattleMobaPlayerState>(ps);
					if (pstate->TeamName == thisstate->TeamName) //check for team this controller belongs to
					{
						if (ps->GetPawn())//check if current selected playerstate has a valid pawn
						{
							APlayerController* pc = Cast<APlayerController>(UGameplayStatics::GetPlayerController(this, pstate->Pi));
							FTimerHandle handle;
							FTimerDelegate TimerDelegate;

							//set view target
							TimerDelegate.BindLambda([this, pc]()
							{
								this->SetViewTargetWithBlend(pc, 2.0f);
								//GEngine->AddOnScreenDebugMessage(-1, 10.0f, FColor::Emerald, FString::Printf(TEXT("Current player is %s"), ((*pstate->GetPawn()->GetFName().ToString()))));
							});
							this->GetWorldTimerManager().SetTimer(handle, TimerDelegate, 0.02f, false);
							break;
						}
					}
				}
			}
		}

		FTimerHandle handle;
		FTimerDelegate TimerDelegate;

		//Possess a pawn
		TimerDelegate.BindLambda([this, thisGameMode, thisstate]()
		{
			thisGameMode->RespawnRequested(this, thisstate->SpawnTransform);
		});
		this->GetWorldTimerManager().SetTimer(handle, TimerDelegate, 27.0f, false);
	}
}