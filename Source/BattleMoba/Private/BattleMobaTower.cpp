// Fill out your copyright notice in the Description page of Project Settings.


#include "BattleMobaTower.h"
#include "Engine.h"
#include "Components/StaticMeshComponent.h"
#include "Net/UnrealNetwork.h"
#include "Components/WidgetComponent.h"
#include "Blueprint/WidgetTree.h"
#include "Components/TextBlock.h"

void ABattleMobaTower::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	DOREPLIFETIME(ABattleMobaTower, Health);
}

// Sets default values
ABattleMobaTower::ABattleMobaTower()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	Root = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));
	Root->SetupAttachment(RootComponent);

	TowerMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StaticMesh"));
	TowerMesh->SetRelativeLocation(FVector(0.0f, 0.0f, 0.0f));
	TowerMesh->SetMobility(EComponentMobility::Static);
	TowerMesh->SetCollisionProfileName("BlockAll");

	Health = MaxHealth;
	SetReplicates(true);

	//WidgetComponent
	W_Health = CreateDefaultSubobject<UWidgetComponent>(TEXT("W_Health"));
	W_Health->SetRelativeLocation(FVector(0.0f, 0.0f, 250.0f));
	W_Health->InitWidget();

	W_Health->SetWidgetSpace(EWidgetSpace::Screen);
	W_Health->SetDrawAtDesiredSize(true);
	W_Health->SetGenerateOverlapEvents(false);
	
}

// Called when the game starts or when spawned
void ABattleMobaTower::BeginPlay()
{
	Super::BeginPlay();
	
	Health = MaxHealth;

	W_DisplayHealth = Cast<UUserWidget>(W_Health->GetUserWidgetObject());
}

void ABattleMobaTower::OnRep_UpdateHealth()
{
	if (W_DisplayHealth)
	{
		GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Blue, FString::Printf(TEXT("Player %s with %s Widget"), *GetDebugName(this), *W_DisplayHealth->GetFName().ToString()));
		const FName hptext = FName(TEXT("HealthText"));
		UTextBlock* HealthText = (UTextBlock*)(W_DisplayHealth->WidgetTree->FindWidget(hptext));

		if (HealthText)
		{
			FString TheFloatStr = FString::SanitizeFloat(this->Health);

			HealthText->SetText(FText::FromString(TheFloatStr));
		}
	}
}

float ABattleMobaTower::TakeDamage(float value, FDamageEvent const & DamageEvent, AController * EventInstigator, AActor * DamageCauser)
{
	Health = FMath::Clamp(Health - value, 0.0f, MaxHealth);
	return Health;
}

// Called every frame
void ABattleMobaTower::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

