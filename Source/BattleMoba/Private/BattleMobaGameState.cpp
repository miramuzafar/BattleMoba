// Fill out your copyright notice in the Description page of Project Settings.


#include "BattleMobaGameState.h"
#include "Net/UnrealNetwork.h"
#include "Kismet/GameplayStatics.h"
#include "BattleMobaCharacter.h"
#include "BattleMobaPlayerState.h"
#include "BattleMobaCTF.h"
#include "Blueprint/WidgetTree.h"
#include "Components/TextBlock.h"
#include "Components/ProgressBar.h"
#include "Components/WidgetComponent.h"

void ABattleMobaGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ABattleMobaGameState, TeamA);
	DOREPLIFETIME(ABattleMobaGameState, TeamB);
	DOREPLIFETIME(ABattleMobaGameState, TeamKillA);
	DOREPLIFETIME(ABattleMobaGameState, TeamKillB);
	DOREPLIFETIME(ABattleMobaGameState, Timer);
	DOREPLIFETIME(ABattleMobaGameState, CurrentTime);
	DOREPLIFETIME(ABattleMobaGameState, Winner);
}

void ABattleMobaGameState::SetTowerWidgetColors(ABattleMobaCTF* cf)
{
	if (cf->ActivePlayer != nullptr)
	{
		//GEngine->AddOnScreenDebugMessage(-1, 10.0f, FColor::Cyan, FString::Printf(TEXT("Actor tName %s"), ((*cf->ActivePlayer->GetFName().ToString()))));
		UUserWidget* HPWidget = Cast<UUserWidget>(cf->W_ValControl->GetUserWidgetObject());
		if (HPWidget)
		{
			const FName hptext = FName(TEXT("ValText"));
			UTextBlock* HealthText = (UTextBlock*)(HPWidget->WidgetTree->FindWidget(hptext));

			const FName hpbar = FName(TEXT("PBar"));
			UProgressBar* PBar = (UProgressBar*)(HPWidget->WidgetTree->FindWidget(hpbar));

			if (PBar->Percent <= 0.0f)
			{
				//get playerstate from local player controller
				ABattleMobaPlayerState* thisPS = Cast<ABattleMobaPlayerState>(UGameplayStatics::GetPlayerController(GetWorld(), 0)->PlayerState);

				//get playerstate from current tower owner
				ABattleMobaPlayerState* activePS = Cast<ABattleMobaPlayerState>(cf->ActivePlayer->GetPlayerState());

				//if current controller is current active player that owned tower
				if (thisPS == activePS)
				{
					//Change progressbar color to blue
					PBar->SetFillColorAndOpacity(FLinearColor(0.5f, 1.0f, 0.0f));
					if (HealthText)
					{
						HealthText->SetColorAndOpacity(FLinearColor(0.5f, 1.0f, 0.0f));
					}				
				}
				else if (thisPS->TeamName == activePS->TeamName)//if current owner is on the same team as current local player
				{
					PBar->SetFillColorAndOpacity(FLinearColor(0.5f, 1.0f, 0.0f));
					if (HealthText)
					{
						HealthText->SetColorAndOpacity(FLinearColor(0.5f, 1.0f, 0.0f));
					}
				}
				else //if its an enemy, change tower to red locally
				{
					PBar->SetFillColorAndOpacity(FLinearColor(1.0f, 0.0f, 0.0f));
					if (HealthText)
					{
						HealthText->SetColorAndOpacity(FLinearColor(1.0f, 0.0f, 0.0f));
					}
				}
			}
		}
		//GEngine->AddOnScreenDebugMessage(-1, 10.0f, FColor::Red, FString::Printf(TEXT("Actor is invalid")));
	}
}

//void ABattleMobaGameState::OnRep_TeamKillCountA()
//{
//
//}
//
//void ABattleMobaGameState::OnRep_TeamKillCountB()
//{
//
//}
