// Fill out your copyright notice in the Description page of Project Settings.


#include "BattleMobaCTF.h"
#include "Net/UnrealNetwork.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SphereComponent.h"
#include "Blueprint/WidgetTree.h"
#include "Components/TextBlock.h"
#include "Components/WidgetComponent.h"
#include "TimerManager.h"
#include "Styling/SlateColor.h"

#include "BattleMobaCharacter.h"

void ABattleMobaCTF::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ABattleMobaCTF, FlagTimer);
	DOREPLIFETIME(ABattleMobaCTF, ControllerTeam);
	DOREPLIFETIME(ABattleMobaCTF, OverlappedPlayer);
	DOREPLIFETIME(ABattleMobaCTF, RadiantControl);
	DOREPLIFETIME(ABattleMobaCTF, DireControl);
	DOREPLIFETIME(ABattleMobaCTF, isCompleted);
}

// Sets default values
ABattleMobaCTF::ABattleMobaCTF()
{
	/*TriggerSphere = CreateDefaultSubobject<USphereComponent>(TEXT("Radius"));
	SetRootComponent(TriggerSphere);
	TriggerSphere->bVisualizeComponent = true;
	TriggerSphere->SetGenerateOverlapEvents(true);

	TriggerSphere->SetSphereRadius(RadiusSize, true);*/
	
	OnActorBeginOverlap.AddDynamic(this, &ABattleMobaCTF::OnOverlapBegin);
	OnActorEndOverlap.AddDynamic(this, &ABattleMobaCTF::OnOverlapEnd);

	/*TriggerSphere->OnComponentBeginOverlap.AddDynamic(this, &ABattleMobaCF::OnOverlapBegin);
	TriggerSphere->OnComponentEndOverlap.AddDynamic(this, &ABattleMobaCF::OnOverlapEnd);*/


	MeshFlag = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Flag"));
	MeshFlag->SetupAttachment(RootComponent);
	MeshFlag->SetCollisionProfileName("BlockAll");
	MeshFlag->SetGenerateOverlapEvents(false);


	W_ValControl = CreateDefaultSubobject<UWidgetComponent>(TEXT("W_ValControl"));
	W_ValControl->SetupAttachment(RootComponent);
	W_ValControl->SetRelativeLocation(FVector(0.0f, 0.0f, 80.0f));
	W_ValControl->InitWidget();

	W_ValControl->SetWidgetSpace(EWidgetSpace::Screen);
	W_ValControl->SetDrawAtDesiredSize(true);
	W_ValControl->SetGenerateOverlapEvents(false);

	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

}

// Called when the game starts or when spawned
void ABattleMobaCTF::BeginPlay()
{
	Super::BeginPlay();

	this->GetWorldTimerManager().SetTimer(FlagTimer, this, &ABattleMobaCTF::TimerFunction, ControllingSpeed, true, 1.0f);
}

void ABattleMobaCTF::OnOverlapBegin(AActor * OverlappedActor, AActor * OtherActor)
{
	if (OtherActor && (OtherActor != this))
	{
		ABattleMobaCharacter* pb = Cast<ABattleMobaCharacter>(OtherActor);
		if (pb)
		{
			//		set CTFentering to true to add integer of a team in the sphere
			pb->CTFentering = true;
		}
		
	}
}

void ABattleMobaCTF::OnOverlapEnd(AActor * OverlappedActor, AActor * OtherActor)
{
	if (OtherActor && (OtherActor != this))
	{
		ABattleMobaCharacter* pe = Cast<ABattleMobaCharacter>(OtherActor);
		if (pe)
		{
			//		set CTFentering to true to deduct integer of a team in the sphere
			pe->CTFentering = false;
		}
	}
}

void ABattleMobaCTF::OnRep_Val()
{
	
	UUserWidget* HPWidget = Cast<UUserWidget>(W_ValControl->GetUserWidgetObject());
	if (HPWidget)
	{
		const FName hptext = FName(TEXT("ValText"));
		UTextBlock* HealthText = (UTextBlock*)(HPWidget->WidgetTree->FindWidget(hptext));

		/*const FName hpbar = FName(TEXT("HPBar"));
		UProgressBar* HealthBar = (UProgressBar*)(HPWidget->WidgetTree->FindWidget(hpbar));*/

		if (HealthText)
		{
			if (this->valDire <= 0 && this->valRadiant > 0)
			{
				FString TheFloatStr = FString::SanitizeFloat(this->valRadiant);
				HealthText->SetText(FText::FromString(TheFloatStr));
				HealthText->SetColorAndOpacity(FLinearColor(0.0f, 0.0f, 1.0f));
			}

			else if (this->valRadiant <= 0 && this->valDire > 0)
			{
				FString TheFloatStr = FString::SanitizeFloat(this->valDire);
				HealthText->SetText(FText::FromString(TheFloatStr));
				HealthText->SetColorAndOpacity(FLinearColor(1.0f, 0.0f, 0.0f));
			}
			
			else
			{
				FString TheFloatStr = FString::SanitizeFloat(0.0f);
				HealthText->SetText(FText::FromString(TheFloatStr));
				HealthText->SetColorAndOpacity(FLinearColor(1.0f, 1.0f, 1.0f));
			}

			
			//HealthBar->SetPercent(FMath::Clamp(this->Health / 100.0f, 0.0f, 1.0f));
		}
	}
}

void ABattleMobaCTF::TimerFunction()
{
	if (!isCompleted)
	{
		//		check overlapping actor in CTF sphere for every second
		this->GetOverlappingActors(this->OverlappedPlayer, ABattleMobaCharacter::StaticClass());
		int arrLength = this->OverlappedPlayer.Num();

		for (uint8 i = 0; i < arrLength; ++i)
		{
			ActivePlayer = Cast<ABattleMobaCharacter>(this->OverlappedPlayer[i]);
			
			if (ActivePlayer)
			{
				if (ActivePlayer->CTFentering == true)
				{
					if (ActivePlayer->TeamName == "Radiant" && this->RadiantControl >= 0)
					{
						this->RadiantControl = this->RadiantControl + 1;
					}

					else if (ActivePlayer->TeamName == "Dire" && this->DireControl >= 0)
					{
						this->DireControl = this->DireControl + 1;
					}

					ActivePlayer->ControlFlagMode(this);
				}
				
				else if (ActivePlayer->CTFentering == false)
				{
					if (ActivePlayer->TeamName == "Radiant" && this->RadiantControl >= 1)
					{
						this->RadiantControl = this->RadiantControl - 1;
					}

					else if (ActivePlayer->TeamName == "Dire" && this->DireControl >= 1)
					{
						this->DireControl = this->DireControl - 1;
					}

					ActivePlayer->ControlFlagMode(this);
				}
			}
		}
	}

	else
	{
		//		stop CTF timer when it is fully controlled by a team
		this->GetWorldTimerManager().ClearTimer(FlagTimer);
	}
}




