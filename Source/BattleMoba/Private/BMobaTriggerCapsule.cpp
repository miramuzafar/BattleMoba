// Fill out your copyright notice in the Description page of Project Settings.


#include "BMobaTriggerCapsule.h"
#include "Net/UnrealNetwork.h"
#include "Blueprint/WidgetTree.h"
#include "Components/TextBlock.h"
#include "Components/ProgressBar.h"
#include "Components/WidgetComponent.h"
#include "TimerManager.h"
#include "Kismet/KismetMathLibrary.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SphereComponent.h"

/////////////////////////////////
#include "BattleMobaCharacter.h"


void ABMobaTriggerCapsule::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ABMobaTriggerCapsule, FlagTimer);
	DOREPLIFETIME(ABMobaTriggerCapsule, TeamName);
}

ABMobaTriggerCapsule::ABMobaTriggerCapsule()
{
	//Register Events
	OnActorBeginOverlap.AddDynamic(this, &ABMobaTriggerCapsule::OnOverlapBegin);
	OnActorEndOverlap.AddDynamic(this, &ABMobaTriggerCapsule::OnOverlapEnd);

	//Mesh
	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("DisplayMesh"));
	Mesh->SetupAttachment(RootComponent);
	Mesh->SetCollisionProfileName("BlockAll");
	Mesh->SetGenerateOverlapEvents(false);

	//Widget
	//WidgetComponent
	W_Val = CreateDefaultSubobject<UWidgetComponent>(TEXT("W_ValueDisplay"));
	W_Val->SetupAttachment(RootComponent);
	W_Val->SetRelativeLocation(FVector(0.000000f, 0.0f, 100.0f));
	W_Val->InitWidget();

	W_Val->SetWidgetSpace(EWidgetSpace::Screen);
	W_Val->SetDrawAtDesiredSize(true);
	W_Val->SetGenerateOverlapEvents(false);
}

bool ABMobaTriggerCapsule::ChangeUIMulticast_Validate(ABattleMobaCharacter * actor)
{
	return true;
}

void ABMobaTriggerCapsule::ChangeUIMulticast_Implementation(ABattleMobaCharacter * actor)
{
}

bool ABMobaTriggerCapsule::ChangeUI_Validate(ABattleMobaCharacter * actor)
{
	return true;
}

void ABMobaTriggerCapsule::ChangeUI_Implementation(ABattleMobaCharacter * actor)
{
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

		const FName hpbar = FName(TEXT("PBar"));
		UProgressBar* PBar = (UProgressBar*)(HPWidget->WidgetTree->FindWidget(hpbar));

		if (HealthText)
		{
			FString TheFloatStr = FString::SanitizeFloat(this->val);

			HealthText->SetText(FText::FromString(TheFloatStr));
			PBar->SetPercent(FMath::Clamp(this->val / 100.0f, 0.0f, 1.0f));
		}
	}
}

void ABMobaTriggerCapsule::OnOverlapBegin(AActor* OverlappedActor, AActor* OtherActor)
{
	if (OtherActor && (OtherActor != this))
	{
		ABattleMobaCharacter* pc = Cast<ABattleMobaCharacter>(OtherActor);
		GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Orange, FString::Printf(TEXT("Overlapping %s"), *UKismetSystemLibrary::GetDisplayName(OtherActor)));
		if (pc)
		{
			//if (TeamName == "")
			//{
			//	TeamName = pc->TeamName;
			//	/*UUserWidget* HPWidget = Cast<UUserWidget>(W_Val->GetUserWidgetObject());
			//	if (HPWidget)
			//	{
			//		const FName hpbar = FName(TEXT("PBar"));
			//		UProgressBar* PBar = (UProgressBar*)(HPWidget->WidgetTree->FindWidget(hpbar));

			//		if (PBar)
			//		{
			//			FSlateBrush newBrush;
			//			if (pc->IsLocallyControlled())
			//			{
			//				newBrush.TintColor = FLinearColor(0.0f, 0.0f, 0.5f);
			//			}
			//			else
			//			{
			//				newBrush.TintColor = FLinearColor(1.0f, 0.0f, 0.0f);
			//			}
			//			PBar->WidgetStyle.SetBackgroundImage(newBrush);
			//		}
			//	}*/
			//}
			TeamName = pc->TeamName;
			pc->SafeZone(this);
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
			pc->SafeZone(this);
		}
	}
}
