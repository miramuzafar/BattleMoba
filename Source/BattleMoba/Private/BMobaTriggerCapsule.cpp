// Fill out your copyright notice in the Description page of Project Settings.


#include "BMobaTriggerCapsule.h"
#include "Net/UnrealNetwork.h"
#include "Blueprint/WidgetTree.h"
#include "Components/TextBlock.h"
#include "Components/WidgetComponent.h"
#include "TimerManager.h"

/////////////////////////////////
#include "BattleMobaCharacter.h"


void ABMobaTriggerCapsule::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
}

ABMobaTriggerCapsule::ABMobaTriggerCapsule()
{
	//Register Events
	OnActorBeginOverlap.AddDynamic(this, &ABMobaTriggerCapsule::OnOverlapBegin);
	OnActorEndOverlap.AddDynamic(this, &ABMobaTriggerCapsule::OnOverlapEnd);

	//Widget
	//WidgetComponent
	W_Val = CreateDefaultSubobject<UWidgetComponent>(TEXT("W_ValueDisplay"));
	W_Val->SetupAttachment(RootComponent);
	W_Val->SetRelativeLocation(FVector(0.000000f, 0.0f, 100.0f));
	W_Val->InitWidget();

	W_Val->SetWidgetSpace(EWidgetSpace::Screen);
	W_Val->SetDrawAtDesiredSize(true);
	//W_DamageOutput->SetVisibility(false);
	W_Val->SetGenerateOverlapEvents(false);
}

void ABMobaTriggerCapsule::BeginPlay()
{
	Super::BeginPlay();
}

void ABMobaTriggerCapsule::OnRep_Val()
{
	UUserWidget* HPWidget = Cast<UUserWidget>(W_Val->GetUserWidgetObject());
	if (HPWidget)
	{
		const FName hptext = FName(TEXT("ValText"));
		UTextBlock* HealthText = (UTextBlock*)(HPWidget->WidgetTree->FindWidget(hptext));

		/*const FName hpbar = FName(TEXT("HPBar"));
		UProgressBar* HealthBar = (UProgressBar*)(HPWidget->WidgetTree->FindWidget(hpbar));*/

		if (HealthText)
		{
			FString TheFloatStr = FString::SanitizeFloat(this->val);

			HealthText->SetText(FText::FromString(TheFloatStr));
			//HealthBar->SetPercent(FMath::Clamp(this->Health / 100.0f, 0.0f, 1.0f));
		}
	}
}

void ABMobaTriggerCapsule::OnOverlapBegin(AActor* OverlappedActor, AActor* OtherActor)
{
	if (OtherActor && (OtherActor != this))
	{
		ABattleMobaCharacter* pc = Cast<ABattleMobaCharacter>(OtherActor);
		if (pc)
		{
			pc->SafeZone(this, &FlagTimer);
		}
	}
}

void ABMobaTriggerCapsule::OnOverlapEnd(AActor* OverlappedActor, AActor* OtherActor)
{
	if (OtherActor && (OtherActor != this))
	{
		ABattleMobaCharacter* pc = Cast<ABattleMobaCharacter>(OtherActor);
		if (pc)
		{
			pc->SafeZone(this, &FlagTimer);
		}
	}
}
