// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#include "BattleMobaCharacter.h"
#include "Engine.h"
#include "Net/UnrealNetwork.h"
#include "TimerManager.h"
#include "HeadMountedDisplayFunctionLibrary.h"
#include "Components/WidgetComponent.h"
#include "Blueprint/WidgetTree.h"
#include "Components/TextBlock.h"
#include "Components/ProgressBar.h"
#include "DrawDebugHelpers.h"
#include "Kismet/GameplayStatics.h"
#include "Camera/CameraComponent.h"
#include "Styling/SlateColor.h"
#include "Components/PrimitiveComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/InputComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Controller.h"
#include "GameFramework/SpringArmComponent.h"
#include "Kismet/KismetStringLibrary.h"
#include "Kismet/KismetMathLibrary.h"
#include "Components/SkinnedMeshComponent.h"
#include "Math/Rotator.h"
#include "Animation/BlendSpace1D.h"
#include "Animation/AnimSingleNodeInstance.h"

//////////////////////////////////////////////////////////////////////////
// ABattleMobaCharacter
#include "InputLibrary.h"
#include "BattleMobaAnimInstance.h"
#include "BattleMobaPC.h"
#include "DestructibleTower.h"
#include "BattleMobaGameState.h"
#include "BattleMobaPlayerState.h"
#include "BattleMobaGameMode.h"
#include "BMobaTriggerCapsule.h"
#include "BattleMobaCTF.h"


void ABattleMobaCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ABattleMobaCharacter, TeamName);
	DOREPLIFETIME(ABattleMobaCharacter, PlayerName);
	DOREPLIFETIME(ABattleMobaCharacter, Health);
	DOREPLIFETIME(ABattleMobaCharacter, Stamina);
	DOREPLIFETIME(ABattleMobaCharacter, IsHit);
	DOREPLIFETIME(ABattleMobaCharacter, InRagdoll);
	DOREPLIFETIME(ABattleMobaCharacter, BoneName);
	DOREPLIFETIME(ABattleMobaCharacter, HitLocation);
	DOREPLIFETIME(ABattleMobaCharacter, AttackSection);
	DOREPLIFETIME(ABattleMobaCharacter, CurrentSection);
	DOREPLIFETIME(ABattleMobaCharacter, TargetHead);
	DOREPLIFETIME(ABattleMobaCharacter, DamageDealers);
	DOREPLIFETIME(ABattleMobaCharacter, Rotate);
	DOREPLIFETIME(ABattleMobaCharacter, AttackerLocation);
	DOREPLIFETIME(ABattleMobaCharacter, CharMesh);
	DOREPLIFETIME(ABattleMobaCharacter, currentTarget);
	DOREPLIFETIME(ABattleMobaCharacter, CounterMoveset);
	DOREPLIFETIME(ABattleMobaCharacter, HitEffect);
	DOREPLIFETIME(ABattleMobaCharacter, CTFteam);
	DOREPLIFETIME(ABattleMobaCharacter, CTFentering);
	DOREPLIFETIME(ABattleMobaCharacter, ActorsToGetGold);
	DOREPLIFETIME(ABattleMobaCharacter, closestActor);
	DOREPLIFETIME(ABattleMobaCharacter, RotateToActor);
	DOREPLIFETIME(ABattleMobaCharacter, IsStunned);
	DOREPLIFETIME(ABattleMobaCharacter, OnSpecialAttack);
	DOREPLIFETIME(ABattleMobaCharacter, AttackTimer);
	DOREPLIFETIME(ABattleMobaCharacter, AttackCol1);
	DOREPLIFETIME(ABattleMobaCharacter, AttackCol2);
	DOREPLIFETIME(ABattleMobaCharacter, AttackCol3);
	DOREPLIFETIME(ABattleMobaCharacter, AttackCol4);
	DOREPLIFETIME(ABattleMobaCharacter, AttackCol5);
	DOREPLIFETIME(ABattleMobaCharacter, AttackCol6);
	DOREPLIFETIME(ABattleMobaCharacter, bApplyHitTrace);
}

ABattleMobaCharacter::ABattleMobaCharacter()
{
	// Create a outline
	Outline = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Outline"));
	Outline->SetupAttachment(RootComponent);
	Outline->SetRelativeLocation(FVector(0.00f, 0.000000f, -98.000000f));
	Outline->SetVisibility(false);

	//this->GetMesh()->SetVisibility(false);
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);

	// Create Sphere Component
	ViewDistanceCol = CreateDefaultSubobject<USphereComponent>(TEXT("ViewDistCol"));
	ViewDistanceCol->SetupAttachment(RootComponent);

	// set our turn rates for input
	BaseTurnRate = 45.f;
	BaseLookUpRate = 45.f;

	// Don't rotate when the controller rotates. Let that just affect the camera.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = true; // Character moves in the direction of input...	
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 540.0f, 0.0f); // ...at this rotation rate
	GetCharacterMovement()->JumpZVelocity = 600.f;
	GetCharacterMovement()->AirControl = 0.2f;

	// Create a camera boom (pulls in towards the player if there is a collision)
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 300.0f; // The camera follows at this distance behind the character	
	CameraBoom->bUsePawnControlRotation = true; // Rotate the arm based on the controller

	// Create a follow camera
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName); // Attach the camera to the end of the boom and let the boom adjust to match the controller orientation
	FollowCamera->bUsePawnControlRotation = false; // Camera does not rotate relative to arm

	BaseArrow = CreateDefaultSubobject<UArrowComponent>(TEXT("BaseArrow"));
	BaseArrow->SetupAttachment(RootComponent);
	BaseArrow->SetRelativeLocation(FVector(0.00f, 0.000000f, 0.000000f));

	// Note: The skeletal mesh and anim blueprint references on the Mesh component (inherited from Character) 
	// are set in the derived blueprint asset named MyCharacter (to avoid direct content references in C++)

	LeftKickCol = CreateDefaultSubobject<UBoxComponent>(TEXT("LeftKickCol"));
	LeftKickCol->SetupAttachment(GetMesh(), "calf_twist_01_l");
	LeftKickCol->SetRelativeLocation(FVector(-5.0f, 0.0f, 0.0f));
	LeftKickCol->SetRelativeRotation(FRotator(0.0f, 180.0f, 90.0f));
	LeftKickCol->SetBoxExtent(FVector(20.0f, 5.0f, 5.0f));
	LeftKickCol->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);

	LKC1 = CreateDefaultSubobject<UBoxComponent>(TEXT("LKC1"));
	LKC1->SetupAttachment(LeftKickCol);
	LKC1->SetRelativeLocation(FVector(-15.0f, 0.0f, 0.0f));
	LKC1->SetBoxExtent(FVector(3.0f, 5.0f, 5.0f));
	LKC1->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);

	LKC2 = CreateDefaultSubobject<UBoxComponent>(TEXT("LKC2"));
	LKC2->SetupAttachment(LeftKickCol);
	LKC2->SetRelativeLocation(FVector(-8.0f, 0.0f, 0.0f));
	LKC2->SetBoxExtent(FVector(3.0f, 5.0f, 5.0f));
	LKC2->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);

	LKC3 = CreateDefaultSubobject<UBoxComponent>(TEXT("LKC3"));
	LKC3->SetupAttachment(LeftKickCol);
	LKC3->SetRelativeLocation(FVector(-1.0f, 0.0f, 0.0f));
	LKC3->SetBoxExtent(FVector(3.0f, 5.0f, 5.0f));
	LKC3->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);

	LKC4 = CreateDefaultSubobject<UBoxComponent>(TEXT("LKC4"));
	LKC4->SetupAttachment(LeftKickCol);
	LKC4->SetRelativeLocation(FVector(5.0f, 0.0f, 0.0f));
	LKC4->SetBoxExtent(FVector(3.0f, 5.0f, 5.0f));
	LKC4->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);

	LKC5 = CreateDefaultSubobject<UBoxComponent>(TEXT("LKC5"));
	LKC5->SetupAttachment(LeftKickCol);
	LKC5->SetRelativeLocation(FVector(10.0f, 0.0f, 0.0f));
	LKC5->SetBoxExtent(FVector(3.0f, 5.0f, 5.0f));
	LKC5->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);

	LKC6 = CreateDefaultSubobject<UBoxComponent>(TEXT("LKC6"));
	LKC6->SetupAttachment(LeftKickCol);
	LKC6->SetRelativeLocation(FVector(16.0f, 0.0f, 0.0f));
	LKC6->SetBoxExtent(FVector(3.0f, 5.0f, 5.0f));
	LKC6->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);

	RightKickCol = CreateDefaultSubobject<UBoxComponent>(TEXT("RightKickCol"));
	RightKickCol->SetupAttachment(GetMesh(), "calf_twist_01_r");
	RightKickCol->SetRelativeLocation(FVector(5.0f, 0.0f, 0.0f));
	RightKickCol->SetRelativeRotation(FRotator(0.0f, 0.0f, 90.0f));
	RightKickCol->SetBoxExtent(FVector(20.0f, 5.0f, 5.0f));
	RightKickCol->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);

	RKC1 = CreateDefaultSubobject<UBoxComponent>(TEXT("RKC1"));
	RKC1->SetupAttachment(RightKickCol);
	RKC1->SetRelativeLocation(FVector(-15.0f, 0.0f, 0.0f));
	RKC1->SetBoxExtent(FVector(3.0f, 5.0f, 5.0f));
	RKC1->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);

	RKC2 = CreateDefaultSubobject<UBoxComponent>(TEXT("RKC2"));
	RKC2->SetupAttachment(RightKickCol);
	RKC2->SetRelativeLocation(FVector(-8.0f, 0.0f, 0.0f));
	RKC2->SetBoxExtent(FVector(3.0f, 5.0f, 5.0f));
	RKC2->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);

	RKC3 = CreateDefaultSubobject<UBoxComponent>(TEXT("RKC3"));
	RKC3->SetupAttachment(RightKickCol);
	RKC3->SetRelativeLocation(FVector(-1.0f, 0.0f, 0.0f));
	RKC3->SetBoxExtent(FVector(3.0f, 5.0f, 5.0f));
	RKC3->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);

	RKC4 = CreateDefaultSubobject<UBoxComponent>(TEXT("RKC4"));
	RKC4->SetupAttachment(RightKickCol);
	RKC4->SetRelativeLocation(FVector(5.0f, 0.0f, 0.0f));
	RKC4->SetBoxExtent(FVector(3.0f, 5.0f, 5.0f));
	RKC4->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);

	RKC5 = CreateDefaultSubobject<UBoxComponent>(TEXT("RKC5"));
	RKC5->SetupAttachment(RightKickCol);
	RKC5->SetRelativeLocation(FVector(10.0f, 0.0f, 0.0f));
	RKC5->SetBoxExtent(FVector(3.0f, 5.0f, 5.0f));
	RKC5->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);

	RKC6 = CreateDefaultSubobject<UBoxComponent>(TEXT("RKC6"));
	RKC6->SetupAttachment(RightKickCol);
	RKC6->SetRelativeLocation(FVector(16.0f, 0.0f, 0.0f));
	RKC6->SetBoxExtent(FVector(3.0f, 5.0f, 5.0f));
	RKC6->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);

	LKickArrow = CreateDefaultSubobject<UArrowComponent>(TEXT("LKickArrow"));
	LKickArrow->SetupAttachment(LeftKickCol);
	LKickArrow->SetRelativeRotation(FRotator(0.0f, 0.0f, 0.0f));

	RKickArrow = CreateDefaultSubobject<UArrowComponent>(TEXT("RKickArrow"));
	RKickArrow->SetupAttachment(RightKickCol);
	RKickArrow->SetRelativeRotation(FRotator(0.0f, 0.0f, 0.0f));

	LeftPunchCol = CreateDefaultSubobject<UBoxComponent>(TEXT("LeftPunchCol"));
	LeftPunchCol->SetupAttachment(GetMesh(), "hand_l");
	LeftPunchCol->SetRelativeLocation(FVector(0.0f, 0.0f, 0.0f));
	LeftPunchCol->SetRelativeRotation(FRotator(0.0f, 0.0f, 0.0f));
	LeftPunchCol->SetBoxExtent(FVector(20.0f, 5.0f, 5.0f));
	LeftPunchCol->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);

	LPC1 = CreateDefaultSubobject<UBoxComponent>(TEXT("LPC1"));
	LPC1->SetupAttachment(LeftPunchCol);
	LPC1->SetRelativeLocation(FVector(-15.0f, 0.0f, 0.0f));
	LPC1->SetBoxExtent(FVector(3.0f, 5.0f, 5.0f));
	LPC1->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);

	LPC2 = CreateDefaultSubobject<UBoxComponent>(TEXT("LPC2"));
	LPC2->SetupAttachment(LeftPunchCol);
	LPC2->SetRelativeLocation(FVector(-10.0f, 0.0f, 0.0f));
	LPC2->SetBoxExtent(FVector(3.0f, 5.0f, 5.0f));
	LPC2->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);

	LPC3 = CreateDefaultSubobject<UBoxComponent>(TEXT("LPC3"));
	LPC3->SetupAttachment(LeftPunchCol);
	LPC3->SetRelativeLocation(FVector(-5.0f, 0.0f, 0.0f));
	LPC3->SetBoxExtent(FVector(3.0f, 5.0f, 5.0f));
	LPC3->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);

	LPC4 = CreateDefaultSubobject<UBoxComponent>(TEXT("LPC4"));
	LPC4->SetupAttachment(LeftPunchCol);
	LPC4->SetRelativeLocation(FVector(0.0f, 0.0f, 0.0f));
	LPC4->SetBoxExtent(FVector(3.0f, 5.0f, 5.0f));
	LPC4->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);

	LPC5 = CreateDefaultSubobject<UBoxComponent>(TEXT("LPC5"));
	LPC5->SetupAttachment(LeftPunchCol);
	LPC5->SetRelativeLocation(FVector(5.0f, 0.0f, 0.0f));
	LPC5->SetBoxExtent(FVector(3.0f, 5.0f, 5.0f));
	LPC5->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);

	LPC6 = CreateDefaultSubobject<UBoxComponent>(TEXT("LPC6"));
	LPC6->SetupAttachment(LeftPunchCol);
	LPC6->SetRelativeLocation(FVector(10.0f, 0.0f, 0.0f));
	LPC6->SetBoxExtent(FVector(3.0f, 5.0f, 5.0f));
	LPC6->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);

	RightPunchCol = CreateDefaultSubobject<UBoxComponent>(TEXT("RightPunchCol"));
	RightPunchCol->SetupAttachment(GetMesh(), "hand_r");
	RightPunchCol->SetRelativeLocation(FVector(0.0f, 0.0f, 0.0f));
	RightPunchCol->SetRelativeRotation(FRotator(180.0f, 0.0f, 0.0f));
	RightPunchCol->SetBoxExtent(FVector(20.0f, 5.0f, 5.0f));
	RightPunchCol->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);

	RPC1 = CreateDefaultSubobject<UBoxComponent>(TEXT("RPC1"));
	RPC1->SetupAttachment(RightPunchCol);
	RPC1->SetRelativeLocation(FVector(-15.0f, 0.0f, 0.0f));
	RPC1->SetBoxExtent(FVector(3.0f, 5.0f, 5.0f));
	RPC1->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);

	RPC2 = CreateDefaultSubobject<UBoxComponent>(TEXT("RPC2"));
	RPC2->SetupAttachment(RightPunchCol);
	RPC2->SetRelativeLocation(FVector(-10.0f, 0.0f, 0.0f));
	RPC2->SetBoxExtent(FVector(3.0f, 5.0f, 5.0f));
	RPC2->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);

	RPC3 = CreateDefaultSubobject<UBoxComponent>(TEXT("RPC3"));
	RPC3->SetupAttachment(RightPunchCol);
	RPC3->SetRelativeLocation(FVector(-5.0f, 0.0f, 0.0f));
	RPC3->SetBoxExtent(FVector(3.0f, 5.0f, 5.0f));
	RPC3->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);

	RPC4 = CreateDefaultSubobject<UBoxComponent>(TEXT("RPC4"));
	RPC4->SetupAttachment(RightPunchCol);
	RPC4->SetRelativeLocation(FVector(0.0f, 0.0f, 0.0f));
	RPC4->SetBoxExtent(FVector(3.0f, 5.0f, 5.0f));
	RPC4->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);

	RPC5 = CreateDefaultSubobject<UBoxComponent>(TEXT("RPC5"));
	RPC5->SetupAttachment(RightPunchCol);
	RPC5->SetRelativeLocation(FVector(5.0f, 0.0f, 0.0f));
	RPC5->SetBoxExtent(FVector(3.0f, 5.0f, 5.0f));
	RPC5->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);

	RPC6 = CreateDefaultSubobject<UBoxComponent>(TEXT("RPC6"));
	RPC6->SetupAttachment(RightPunchCol);
	RPC6->SetRelativeLocation(FVector(10.0f, 0.0f, 0.0f));
	RPC6->SetBoxExtent(FVector(3.0f, 5.0f, 5.0f));
	RPC6->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);

	LPunchArrow = CreateDefaultSubobject<UArrowComponent>(TEXT("LPunchArrow"));
	LPunchArrow->SetupAttachment(GetMesh(), "lowerarm_l");

	RPunchArrow = CreateDefaultSubobject<UArrowComponent>(TEXT("RPunchArrow"));
	RPunchArrow->SetupAttachment(GetMesh(), "lowerarm_r");
	RPunchArrow->SetRelativeRotation(FRotator(0, -180, 0));

	//WidgetComponent
	W_DamageOutput = CreateDefaultSubobject<UWidgetComponent>(TEXT("W_DamageOutput"));
	W_DamageOutput->SetupAttachment(RootComponent);
	W_DamageOutput->SetRelativeLocation(FVector(0.000000f, 0.0f, 100.0f));
	W_DamageOutput->InitWidget();
	W_DamageOutput->SetVisibility(false);

	W_DamageOutput->SetWidgetSpace(EWidgetSpace::Screen);
	W_DamageOutput->SetDrawSize(FVector2D(500.0f, 500.0f));
	W_DamageOutput->SetDrawAtDesiredSize(true);
	//W_DamageOutput->SetVisibility(false);
	W_DamageOutput->SetGenerateOverlapEvents(false);

	TraceDistance = 20.0f;

	//TargetLock
	ViewDistanceCol->SetSphereRadius(200.0f);

	ViewDistanceCol->OnComponentBeginOverlap.AddDynamic(this, &ABattleMobaCharacter::OnBeginOverlap);
	ViewDistanceCol->OnComponentEndOverlap.AddDynamic(this, &ABattleMobaCharacter::OnEndOverlap);

	
}

//////////////////////////////////////////////////////////////////////////
// Input

void ABattleMobaCharacter::OnRep_Health()
{
	UUserWidget* HPWidget = Cast<UUserWidget>(W_DamageOutput->GetUserWidgetObject());
	if (HPWidget)
	{
		GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Blue, FString::Printf(TEXT("Player %s with %s Widget"), *GetDebugName(this), *HPWidget->GetFName().ToString()));
		const FName hptext = FName(TEXT("HealthText"));
		UTextBlock* HealthText = (UTextBlock*)(HPWidget->WidgetTree->FindWidget(hptext));

		const FName hpbar = FName(TEXT("HPBar"));
		UProgressBar* HealthBar = (UProgressBar*)(HPWidget->WidgetTree->FindWidget(hpbar));

		if (HealthText)
		{
			FString TheFloatStr = FString::SanitizeFloat(this->Health);

			HealthText->SetText(FText::FromString(TheFloatStr));
			HealthBar->SetPercent(FMath::Clamp(this->Health / this->MaxHealth, 0.0f, 1.0f));
		}
	}
	//this->Health = UGestureInputsFunctions::UpdateProgressBarComponent(this->WidgetHUD, "HPBar", "Health", "HP", "Pain Meter", this->Health, this->MaxHealth);

	/*if (this->IsLocallyControlled())
	{
		this->Health = UGestureInputsFunctions::UpdateProgressBarComponent(this->WidgetHUD, "HPBar", "Health", "HP", "Pain Meter", this->Health, this->MaxHealth);
	}*/
	/*if (!this->IsLocallyControlled())
	{
		float Health1 = UGestureInputsFunctions::UpdateProgressBarComponent(this->WidgetHUD, "HPBarMain_1", "Health_1", "HP", "Pain Meter", this->Health, this->MaxHealth);
	}*/
}

void ABattleMobaCharacter::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent)
{
	// Set up gameplay key bindings
	check(PlayerInputComponent);
	//PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ACharacter::Jump);
	//PlayerInputComponent->BindAction("Jump", IE_Released, this, &ACharacter::StopJumping);

	PlayerInputComponent->BindAxis("MoveForward", this, &ABattleMobaCharacter::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &ABattleMobaCharacter::MoveRight);

	// We have 2 versions of the rotation bindings to handle different kinds of devices differently
	// "turn" handles devices that provide an absolute delta, such as a mouse.
	// "turnrate" is for devices that we choose to treat as a rate of change, such as an analog joystick
	PlayerInputComponent->BindAxis("Turn", this, &APawn::AddControllerYawInput);
	PlayerInputComponent->BindAxis("TurnRate", this, &ABattleMobaCharacter::TurnAtRate);
	PlayerInputComponent->BindAxis("LookUp", this, &APawn::AddControllerPitchInput);
	PlayerInputComponent->BindAxis("LookUpRate", this, &ABattleMobaCharacter::LookUpAtRate);

	// handle touch devices
	PlayerInputComponent->BindTouch(IE_Pressed, this, &ABattleMobaCharacter::TouchStarted);
	PlayerInputComponent->BindTouch(IE_Released, this, &ABattleMobaCharacter::TouchStopped);

	// VR headset functionality
	PlayerInputComponent->BindAction("ResetVR", IE_Pressed, this, &ABattleMobaCharacter::OnResetVR);

	//	Camera Shake test input
	PlayerInputComponent->BindAction("TestCam", IE_Released, this, &ABattleMobaCharacter::OnCameraShake);
}

void ABattleMobaCharacter::BeginPlay()
{
	Super::BeginPlay();

	this->GetMesh()->SetSkeletalMesh(CharMesh, false);
	AnimInsta = Cast<UBattleMobaAnimInstance>(this->GetMesh()->GetAnimInstance());

	FTimerHandle handle;
	FTimerDelegate TimerDelegate;

	//set the row boolean to false after finish cooldown timer
	TimerDelegate.BindLambda([this]()
	{
		this->GetMesh()->SetVisibility(true);

		GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Blue, FString::Printf(TEXT("Rotate: %s"), Rotate ? TEXT("true") : TEXT("false")));

		/*APlayerController* pc = Cast<APlayerController>(this->GetController());
		if (pc)
		{
			if (pc->IsLocalPlayerController() && pc->GetNetMode() != ENetMode::NM_DedicatedServer)
			{

				pc->bShowMouseCursor = false;
				pc->SetInputMode(FInputModeGameOnly());
			}
		}*/

	});
	this->GetWorldTimerManager().SetTimer(handle, TimerDelegate, 1.0f, false);

	FString Context;
	for (auto& name : ActionTable->GetRowNames())
	{
		FActionSkill* row = ActionTable->FindRow<FActionSkill>(name, Context);

		if (row)
		{
			row->isOnCD = false;
		}
	}

	for (TActorIterator<ABattleMobaCTF> It(GetWorld()); It; ++It)
	{
		Towers.Add(*It);
	}

	CreateCPHUD();

	AttackTraceParams.AddIgnoredActor(this);

	const FName traceTag("MyTraceTag");
	GetWorld()->DebugDrawTraceTag = traceTag;

	AttackTraceParams.TraceTag = traceTag;
}

float ABattleMobaCharacter::TakeDamage(float Damage, FDamageEvent const & DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	if (this->GetLocalRole() == ROLE_Authority)
	{
		if (DamageCauser != this)
		{
			ABattleMobaCharacter* damageChar = Cast<ABattleMobaCharacter>(DamageCauser);
			ABattleMobaPlayerState* ps = Cast<ABattleMobaPlayerState>(damageChar->GetPlayerState());
			if (this->DamageDealers.Contains(ps))
			{
				this->DamageDealers.RemoveSingle(ps);
			}
			this->DamageDealers.Emplace(ps);

			ServerSpawnEffect(damageChar, this);
			

			if (damageChar->OnSpecialAttack == true)
			{
				HitReactionClient(this, Damage, this->HitReactionMoveset, "NormalHit01");
			}

			else if (damageChar->OnSpecialAttack == false)
			{
				/**		Calculate directional hit detection*/
				FRotator RotDifference = UKismetMathLibrary::NormalizedDeltaRotator(this->GetViewRotation(), UKismetMathLibrary::FindLookAtRotation(this->GetPawnViewLocation(), damageChar->GetActorLocation()));
				//GEngine->AddOnScreenDebugMessage(-1, 10.0f, FColor::Yellow, FString::Printf(TEXT("Rotation Delta: %s"), (*RotDifference.ToString())));

				/**		Get random index from array of section names*/
				FName arr[2] = { "NormalHit01", "NormalHit02" };
				int RandInt = rand() % 2;
				//FName HitSection = arr[RandInt];
				FName HitSection = "NormalHit01";

				// right
				if (UKismetMathLibrary::InRange_FloatFloat(RotDifference.Yaw, -135.0f, -45.0f, true, true))
				{
					HitReactionClient(this, Damage, this->RightHitMoveset, HitSection);
					//GEngine->AddOnScreenDebugMessage(-1, 10.0f, FColor::Yellow, FString::Printf(TEXT("Hit from RIGHT")));
				}

				// front
				else if (UKismetMathLibrary::InRange_FloatFloat(RotDifference.Yaw, -45.0f, 45.0f, true, true))
				{
					HitReactionClient(this, Damage, this->FrontHitMoveset, HitSection);
					//GEngine->AddOnScreenDebugMessage(-1, 10.0f, FColor::Yellow, FString::Printf(TEXT("Hit from FRONT")));
				}

				//	left
				else if (UKismetMathLibrary::InRange_FloatFloat(RotDifference.Yaw, 45.0f, 135.0f, true, true))
				{
					HitReactionClient(this, Damage, this->LeftHitMoveset, HitSection);
					//GEngine->AddOnScreenDebugMessage(-1, 10.0f, FColor::Yellow, FString::Printf(TEXT("Hit from LEFT")));
				}

				//	back
				else
				{
					HitReactionClient(this, Damage, this->BackHitMoveset, HitSection);
					//GEngine->AddOnScreenDebugMessage(-1, 10.0f, FColor::Yellow, FString::Printf(TEXT("Hit from BACK")));
				}

			}
		}
	}
	return 0.0f;
}


void ABattleMobaCharacter::OnConstruction(const FTransform & Transform)
{

}

void ABattleMobaCharacter::CheckSwipeType(EInputType Type, FVector2D Location, TEnumAsByte<ETouchIndex::Type> TouchIndex)
{
	if (UGameplayStatics::GetPlatformName() == "IOS" || UGameplayStatics::GetPlatformName() == "Android")
	{
		//Check for touch pressed
		if (Type == EInputType::Pressed)
		{
			if (TouchIndex == ETouchIndex::Touch1)
			{
				//if current Location is on the left side of screen, set swipe mechanic to move the player, else set to rotate the camera
				if (UInputLibrary::PointOnLeftHalfOfScreen(Location))
				{
					GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Blue, FString::Printf(TEXT("Move")));

					TouchStart = Location;
					MoveTouchIndex = TouchIndex;
					IsPressed = true;
				}
				else if (!UInputLibrary::PointOnLeftHalfOfScreen(Location))
				{
					InitRotateToggle = true;
				}
			}
			else if (TouchIndex == ETouchIndex::Touch2)
			{
				//if current Location is on the left side of screen, set swipe mechanic to move the player, else set to rotate the camera
				if (UInputLibrary::PointOnLeftHalfOfScreen(Location))
				{
					GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Blue, FString::Printf(TEXT("Move")));

					TouchStart = Location;
					MoveTouchIndex = TouchIndex;
					IsPressed = true;
				}
				else if (!UInputLibrary::PointOnLeftHalfOfScreen(Location))
				{
					InitRotateToggle = true;
				}
			}
		}
		//If the pressed touch is swiping
		else if (Type == EInputType::Hold)
		{
			if (TouchIndex == ETouchIndex::Touch1)
			{
				//if current Location is on the left side of screen, set swipe mechanic to move the player, else set to rotate the camera
				if (UInputLibrary::PointOnLeftHalfOfScreen(Location))
				{
					GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Blue, FString::Printf(TEXT("Move")));

					TouchEnd = Location;
					MoveTouchIndex = TouchIndex;
				}
				else if (!UInputLibrary::PointOnLeftHalfOfScreen(Location))
				{
					RotTouchIndex = TouchIndex;
				}
			}
			if (TouchIndex == ETouchIndex::Touch2)
			{
				//if current Location is on the left side of screen, set swipe mechanic to move the player, else set to rotate the camera
				if (UInputLibrary::PointOnLeftHalfOfScreen(Location))
				{
					GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Blue, FString::Printf(TEXT("Move")));

					TouchEnd = Location;
					MoveTouchIndex = TouchIndex;
				}
				else if (!UInputLibrary::PointOnLeftHalfOfScreen(Location))
				{
					RotTouchIndex = TouchIndex;
				}
			}
		}
		//if the current touch index is being released
		else if (Type == EInputType::Released)
		{
			if (TouchIndex == ETouchIndex::Touch1)
			{
				if (MoveTouchIndex == TouchIndex)
				{
					IsPressed = false;
				}
			}
			if (TouchIndex == ETouchIndex::Touch2)
			{
				if (MoveTouchIndex == TouchIndex)
				{
					IsPressed = false;
				}
			}

		}
	}

}

void ABattleMobaCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (WithinVicinity)
	{
		UInputLibrary::SetUIVisibility(W_DamageOutput, this);
	}

	////////////////Mobile Input/////////////////////////////
	if (this->GetNetMode() != ENetMode::NM_DedicatedServer)
	{
		//Detect Movement Input
		if (IsPressed)
		{
			AddSwipeVectorToMovementInput();
		}
		if (InitRotateToggle)
		{
			AddSwipeVectorToRotationInput();
		}
	}
	////////////////////////////////////////////////////////
	//if (currentTarget != nullptr && Rotate == true)
	//{
	//	if (HasAuthority())
	//	{
	//		if (this->IsLocallyControlled())
	//		{
	//			FRotator RotatorVal = UKismetMathLibrary::FindLookAtRotation(this->GetCapsuleComponent()->GetComponentLocation(), currentTarget->GetActorLocation());
	//			FRotator FinalVal = FRotator(this->GetCapsuleComponent()->GetComponentRotation().Pitch, RotatorVal.Yaw, this->GetCapsuleComponent()->GetComponentRotation().Roll);
	//			FMath::RInterpTo(this->GetCapsuleComponent()->GetComponentRotation(), FinalVal, DeltaTime, 40.0f);
	//			this->SetActorRotation(FinalVal);
	//			RotateToCameraView(FinalVal);
	//			//this->SetActorRotation(FRotator(this->GetActorRotation().Pitch, this->GetControlRotation().Yaw, this->GetActorRotation().Roll));
	//			//
	//			//Set rotate to false
	//			/*FTimerHandle handle;
	//			FTimerDelegate TimerDelegate;
	//			TimerDelegate.BindLambda([this]()
	//			{
	//				Rotate = false;
	//				GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Blue, FString::Printf(TEXT("Rotate: %s"), Rotate ? TEXT("true") : TEXT("false")));
	//			});
	//			this->GetWorldTimerManager().SetTimer(handle, TimerDelegate, 1.0f, false);*/
	//		}
	//	}
	//	else
	//	{
	//		if (this->GetController() != nullptr)
	//		{
	//			/*ServerRotateToCameraView(FRotator(this->GetActorRotation().Pitch, this->GetControlRotation().Yaw, this->GetActorRotation().Roll));
	//			this->SetActorRotation(FRotator(this->GetActorRotation().Pitch, this->GetControlRotation().Yaw, this->GetActorRotation().Roll));*/
	//			FRotator RotatorVal = UKismetMathLibrary::FindLookAtRotation(this->GetCapsuleComponent()->GetComponentLocation(), currentTarget->GetActorLocation());
	//			FRotator FinalVal = FRotator(this->GetCapsuleComponent()->GetComponentRotation().Pitch, RotatorVal.Yaw, this->GetCapsuleComponent()->GetComponentRotation().Roll);
	//			FMath::RInterpTo(this->GetCapsuleComponent()->GetComponentRotation(), FinalVal, DeltaTime, 40.0f);
	//			ServerRotateToCameraView(FinalVal);
	//			this->SetActorRotation(FinalVal);

	//			//Set rotate to false
	//			/*FTimerHandle handle;
	//			FTimerDelegate TimerDelegate;
	//			TimerDelegate.BindLambda([this]()
	//			{
	//				Rotate = false;
	//				GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Blue, FString::Printf(TEXT("Rotate: %s"), Rotate ? TEXT("true") : TEXT("false")));
	//			});
	//			this->GetWorldTimerManager().SetTimer(handle, TimerDelegate, 1.0f, false);
	//			GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Blue, FString::Printf(TEXT("Rotate: %s"), Rotate ? TEXT("true") : TEXT("false")));*/
	//		}
	//	}
	//}

	//FireTrace(this->GetActorLocation(), this->GetActorLocation(), this->GetActorRotation(), true);
}

void ABattleMobaCharacter::AddSwipeVectorToMovementInput()
{
	//Get world direction of swapping
	FVector2D Total = TouchStart - TouchEnd;

	//Get world position
	FVector WorldDirection = FVector(-Total.Y, Total.X, this->GetActorLocation().Z);
	
	//Get Controller rotator so that teh direction will always align with the rotator
	const FRotator Rotation = Controller->GetControlRotation();
	const FRotator YawRotation(0, Rotation.Yaw, 0);

	//rotate a vector from YawRotation
	const FVector Direction = YawRotation.RotateVector(WorldDirection);

	GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Blue, FString::Printf(TEXT("Enable move")));
	
	//Move character based on world direction
	AddMovementInput(Direction, -1.0f);
}

void ABattleMobaCharacter::AddSwipeVectorToRotationInput()
{
	bool temp;
	FVector2D newVect;
	UGameplayStatics::GetPlayerController(this, 0)->GetInputTouchState(RotTouchIndex, newVect.X, newVect.Y, temp);

	if (temp)
	{
		if (!StartRotate)
		{
			StartRotate = true;
			BaseTurnRate = newVect.X; //Set new x velocity
			BaseLookUpRate = newVect.Y;//Set new y velocity
		}
		else
		{
			//Rotate camera based on velocity of the swipe
			AddControllerYawInput((newVect.X - BaseTurnRate) / 5.0f);
			AddControllerPitchInput((newVect.Y - BaseLookUpRate) / 5.0f);

			BaseTurnRate = newVect.X;//Set new x velocity
			BaseLookUpRate = newVect.Y;//Set new y velocity
		}
	}
	else
	{
		StartRotate = false;
		InitRotateToggle = false;//if current touchindex is released
	}
}

bool ABattleMobaCharacter::ServerRotateToCameraView_Validate(FRotator InRot)
{
	return true;
}

void ABattleMobaCharacter::ServerRotateToCameraView_Implementation(FRotator InRot)
{
	RotateToCameraView(InRot);
}

bool ABattleMobaCharacter::RotateToCameraView_Validate(FRotator InRot)
{
	return true;
}

void ABattleMobaCharacter::RotateToCameraView_Implementation(FRotator InRot)
{
	//Multicast rotation
	FMath::RInterpTo(this->GetCapsuleComponent()->GetComponentRotation(), InRot, GetWorld()->GetDeltaSeconds(), 10.0f);
	this->SetActorRotation(InRot);

	//setting up for cooldown properties
	//FTimerHandle handle;
	//FTimerDelegate TimerDelegate;

	////set the row boolean to false after finish cooldown timer
	//TimerDelegate.BindLambda([this]()
	//{
	//	Rotate = false;
	//	GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Blue, FString::Printf(TEXT("Rotate: %s"), Rotate ? TEXT("true") : TEXT("false")));
	//});
	//this->GetWorldTimerManager().SetTimer(handle, TimerDelegate, 1.0f, false);
}

void ABattleMobaCharacter::SetupWidget()
{
	//OnRep_Team();
	SetupStats();

	/*UUserWidget* HPWidget = Cast<UUserWidget>(W_DamageOutput->GetUserWidgetObject());
	if (HPWidget)
	{
		const FName hptext = FName(TEXT("HealthText"));
		UTextBlock* HealthText = (UTextBlock*)(HPWidget->WidgetTree->FindWidget(hptext));

		if (HealthText)
		{
			FString TheFloatStr = FString::SanitizeFloat(this->Health);

			HealthText->SetText(FText::FromString(TheFloatStr));
			if (TeamName == "Radiant")
			{
				HealthText->SetColorAndOpacity(FLinearColor(0.0, 1.0, 0.0, 1.0));
			}
			else
				HealthText->SetColorAndOpacity(FLinearColor(1.0, 0.0, 0.0, 1.0));
		}
	}*/

	//Setup3DWidgetVisibility();
}

void ABattleMobaCharacter::HideHPBar()
{
	if (WithinVicinity)
	{
		UInputLibrary::SetUIVisibility(W_DamageOutput, this);
		/*FHit(ForceInit);

		FVector start = this->GetActorLocation();
		FVector End = UGameplayStatics::GetPlayerCameraManager(this, 0)->GetCameraLocation();
		FCollisionQueryParams CollisionParams;
		CollisionParams.AddIgnoredActor(this);

		if (GetWorld()->LineTraceSingleByChannel(Hit, start, End, ECC_Visibility, CollisionParams))
		{
			W_DamageOutput->GetUserWidgetObject()->SetVisibility(ESlateVisibility::Hidden);
		}
		else
		{
			W_DamageOutput->GetUserWidgetObject()->SetVisibility(ESlateVisibility::HitTestInvisible);
		}*/
	}
}

bool ABattleMobaCharacter::HitReactionServer_Validate(AActor * HitActor, float DamageReceived, UAnimMontage* HitMoveset, FName MontageSection)
{
	return true;
}

void ABattleMobaCharacter::HitReactionServer_Implementation(AActor* HitActor, float DamageReceived, UAnimMontage* HitMoveset, FName MontageSection)
{
	if (this->GetLocalRole() == ROLE_Authority)
	{
		if (HitActor == this)
		{
			HitReactionClient(HitActor, DamageReceived, HitMoveset, MontageSection);
		}
	}
}

bool ABattleMobaCharacter::HitReactionClient_Validate(AActor* HitActor, float DamageReceived, UAnimMontage* HitMoveset, FName MontageSection)
{
	return true;
}

void ABattleMobaCharacter::HitReactionClient_Implementation(AActor* HitActor, float DamageReceived, UAnimMontage* HitMoveset, FName MontageSection)
{
	if (HitActor == this)
	{
		if (this->InRagdoll == false)
		{
			if (this->Health >= 0)
			{
				float Temp = this->Health - DamageReceived;

				/**		Knockout and respawn*/
				if (Temp <= 0.0f)
				{
					//Set Team Kills
					//ABattleMobaGameState* gs = Cast <ABattleMobaGameState>(UGameplayStatics::GetGameState(this));
					//if (gs)
					//{
					//	if (gs->TeamA.Contains(this->GetPlayerState()->GetPlayerName()))
					//	{
					//		gs->TeamKillB += 1;
					//		//gs->OnRep_TeamKillCountB();
					//	}
					//	else if (gs->TeamB.Contains(this->GetPlayerState()->GetPlayerName()))
					//	{
					//		gs->TeamKillA += 1;
					//		//gs->OnRep_TeamKillCountA();
					//	}
					//	GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Blue, FString::Printf(TEXT("Team A : %d, Team B : %d"), gs->TeamKillA, gs->TeamKillB));
					//}
					Temp = 0.0f;
					
					//disable action
					this->ActionEnabled = false;
					this->WithinVicinity = false;

					//ps->Death += 1;

					FTimerHandle handle;
					FTimerDelegate TimerDelegate;

					ABattleMobaGameMode* gm = Cast<ABattleMobaGameMode>(UGameplayStatics::GetGameMode(this));

					//Set player's death count
					ABattleMobaPlayerState* ps = Cast<ABattleMobaPlayerState>(this->GetPlayerState());

					//set the row boolean to false after finish cooldown timer
					TimerDelegate.BindLambda([this, gm, ps]()
					{
						if (gm)
						{
							gm->PlayerKilled(ps, this->DamageDealers.Last(), DamageDealers); //Set current team scores and kills
						}
					});

					//start cooldown the skill
					this->GetWorldTimerManager().SetTimer(handle, TimerDelegate, 0.02f, false);
					
					this->GetMesh()->SetCollisionEnabled(ECollisionEnabled::PhysicsOnly);
					this->GetMesh()->SetCollisionObjectType(ECollisionChannel::ECC_PhysicsBody);
					this->GetMesh()->SetSimulatePhysics(true);
					this->GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
					this->GetCharacterMovement()->DisableMovement();

					if (GetLocalRole() == ROLE_Authority)
					{
						//Start Respawn Timer Count
						gm->StartRespawnTimer(ps);

						this->GetWorld()->GetTimerManager().SetTimer(this->RespawnTimer, this, &ABattleMobaCharacter::RespawnCharacter, 3.0f, false);
					}
				}
				this->Health = Temp;
				this->IsHit = false;

				///**		Force player to face Attacker*/
				//FRotator PlayerRot = UKismetMathLibrary::FindLookAtRotation(this->GetActorLocation(), AttackerLocation);
				//FRotator NewRot = FMath::RInterpTo(this->GetActorRotation(), PlayerRot, GetWorld()->GetDeltaSeconds(), 200.0f);
				//FRotator NewRot2 = FRotator(this->GetActorRotation().Pitch, NewRot.Yaw, this->GetActorRotation().Roll);
				//this->SetActorRotation(NewRot2);

				/**		Play hit reaction animation on hit*/
				//UAnimMontage* CurrentMontage = this->GetMesh()->GetAnimInstance()->GetCurrentActiveMontage();
				//this->GetMesh()->GetAnimInstance()->Montage_Stop(0.001f, CurrentMontage);
				//PlayAnimMontage(HitMoveset, 1.0f, MontageSection);
				this->GetMesh()->GetAnimInstance()->Montage_Play(HitMoveset, 1.0f, EMontagePlayReturnType::MontageLength, 0.0f, true);
				this->GetMesh()->GetAnimInstance()->Montage_JumpToSection(MontageSection);

				OnRep_Health();

				//run clear damage dealers array
				if (this->GetWorldTimerManager().IsTimerActive(this->DealerTimer))
				{
					this->GetWorldTimerManager().ClearTimer(this->DealerTimer);

					if (this->IsLocallyControlled())
					{
						SetActiveSocket(NAME_None);
					}
				}
				this->GetWorldTimerManager().SetTimer(this->DealerTimer, this, &ABattleMobaCharacter::ClearDamageDealers, 5.0f, true);
			}
		}
	}
	UpdateHUD();
}

bool ABattleMobaCharacter::StunPlayerServer_Validate(bool checkStun)
{
	return true;
}

void ABattleMobaCharacter::StunPlayerServer_Implementation(bool checkStun)
{
	if (this->GetLocalRole() == ROLE_Authority)
	{
		StunPlayerClient(checkStun);
	}
	
}

bool ABattleMobaCharacter::StunPlayerClient_Validate(bool checkStun)
{
	return true;
}

void ABattleMobaCharacter::StunPlayerClient_Implementation(bool checkStun)
{
	/**		called in stun hit reaction montage AnimBP*/
	this->IsStunned = checkStun;
}


bool ABattleMobaCharacter::ServerRotateHitActor_Validate(AActor * HitActor, AActor * Attacker)
{
	return true;
}

void ABattleMobaCharacter::ServerRotateHitActor_Implementation(AActor * HitActor, AActor * Attacker)
{
	if (this->GetLocalRole() == ROLE_Authority)
	{
		if (HitActor == this)
		{
			MulticastRotateHitActor(HitActor, Attacker);
		}
	}
	
}

bool ABattleMobaCharacter::MulticastRotateHitActor_Validate(AActor * HitActor, AActor * Attacker)
{
	return true;
}

void ABattleMobaCharacter::MulticastRotateHitActor_Implementation(AActor * HitActor, AActor * Attacker)
{
	/**		Force player to face Attacker*/
	FRotator hitCharRot = UKismetMathLibrary::FindLookAtRotation(HitActor->GetActorLocation(), Attacker->GetActorLocation());
	FRotator NewRot = FMath::RInterpTo(HitActor->GetActorRotation(), hitCharRot, HitActor->GetWorld()->GetDeltaSeconds(), 200.0f);
	FRotator FinalRot = FRotator(HitActor->GetActorRotation().Pitch, NewRot.Yaw, HitActor->GetActorRotation().Roll);
	HitActor->SetActorRotation(FinalRot);
}

bool ABattleMobaCharacter::ServerSpawnEffect_Validate(ABattleMobaCharacter * EmitActor, ABattleMobaCharacter* HitActor)
{
	return true;
}

void ABattleMobaCharacter::ServerSpawnEffect_Implementation(ABattleMobaCharacter * EmitActor, ABattleMobaCharacter* HitActor)
{
	if (this->GetLocalRole() == ROLE_Authority)
	{
		if (HitActor == this)
		{
			MulticastSpawnEffect(EmitActor, HitActor);
		}
	}
}

bool ABattleMobaCharacter::MulticastSpawnEffect_Validate(ABattleMobaCharacter * EmitActor, ABattleMobaCharacter* HitActor)
{
	return true;
}

void ABattleMobaCharacter::MulticastSpawnEffect_Implementation(ABattleMobaCharacter * EmitActor, ABattleMobaCharacter* HitActor)
{
	if (EmitActor->HitEffect != nullptr)
	{
		//GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Orange, FString::Printf(TEXT("Particle Effect Spawned")));

		UGameplayStatics::SpawnEmitterAtLocation(EmitActor->GetWorld(), EmitActor->HitEffect, EmitActor->GetMesh()->GetSocketLocation(EmitActor->ActiveSocket), FRotator::ZeroRotator, false);
	}
}

bool ABattleMobaCharacter::SetActiveSocket_Validate(FName SocketName)
{
	return true;
}
void ABattleMobaCharacter::SetActiveSocket_Implementation(FName SocketName)
{
	if (this->GetLocalRole() == ROLE_Authority)
	{
		MulticastSetActiveSocket(SocketName);
	}
}

bool ABattleMobaCharacter::MulticastSetActiveSocket_Validate(FName SocketName)
{
	return true;
}

void ABattleMobaCharacter::MulticastSetActiveSocket_Implementation(FName SocketName)
{
	if (SocketName != NAME_None)
	{
		this->ActiveSocket = SocketName;
	}
}

void ABattleMobaCharacter::ClearDamageDealers()
{
	if (this->DamageDealers.IsValidIndex(0))
	{
		this->DamageDealers.RemoveAtSwap(0);
	}
	else
		this->GetWorldTimerManager().ClearTimer(this->DealerTimer);
}

void ABattleMobaCharacter::GetButtonSkillAction(FKey Currkeys, FString ButtonName, bool& cooldown, float& CooldownVal)
{
	if (ActionEnabled == true)
	{
		if (GetMesh()->SkeletalMesh != nullptr)
		{
			//Used in error reporting
			FString Context;
			for (auto& name : ActionTable->GetRowNames())
			{
				FActionSkill* row = ActionTable->FindRow<FActionSkill>(name, Context);

				if (row)
				{
					if (row->keys == Currkeys || row->ButtonName == ButtonName)
					{
						//if current skill is using cooldown
						if (row->IsUsingCD && !row->UseTranslate)
						{
							//GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Blue, FString::Printf(TEXT("row->isOnCD: %s"), row->isOnCD ? TEXT("true") : TEXT("false")));

							//if the skill is on cooldown, stop playing the animation, else play the skill animation
							if (row->isOnCD == true)
							{
								//GEngine->AddOnScreenDebugMessage(-1, 10.0f, FColor::Emerald, FString::Printf(TEXT("Current %s skill is on cooldown!!"), ((*name.ToString()))));
								GEngine->AddOnScreenDebugMessage(-1, 10.0f, FColor::Emerald, FString::Printf(TEXT("Current %s skill is on cooldown!!"), ((*name.ToString()))));
								cooldown = row->isOnCD;
								break;
							}
							else if (row->isOnCD == false)
							{
								cooldown = row->isOnCD;
								row->isOnCD = true;
								//GEngine->AddOnScreenDebugMessage(-1, 10.0f, FColor::Emerald, FString::Printf(TEXT("Current key is %s"), ((*row->keys.ToString()))));
								if (row->SkillMoveset != nullptr)
								{
									TargetHead = row->TargetIsHead;
									if (this->IsLocallyControlled())
									{
										DetectNearestTarget(EResult::Cooldown, *row);
										AttackSection = "NormalAttack01";
										//play the animation that visible to all clients
										//ServerExecuteAction(*row, CurrentSection, AttackSection, true);

										//setting up for cooldown properties
										FTimerHandle handle;
										FTimerDelegate TimerDelegate;

										//set the row boolean to false after finish cooldown timer
										TimerDelegate.BindLambda([row, this]()
										{
											UE_LOG(LogTemp, Warning, TEXT("DELAY BEFORE SETTING UP COOLDOWN TO FALSE"));
											row->isOnCD = false;

											//GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Blue, FString::Printf(TEXT("row->isOnCD: %s"), row->isOnCD ? TEXT("true") : TEXT("false")));
										});

										//start cooldown the skill
										this->GetWorldTimerManager().SetTimer(handle, TimerDelegate, row->CDDuration, false);
										CooldownVal = row->CDDuration;
										break;
									}
								}
								break;
							}
						}

						/**		current skill uses translation*/
						else if (row->IsUsingCD && row->UseTranslate)
						{
							if (row->isOnCD == true)
							{
								//GEngine->AddOnScreenDebugMessage(-1, 10.0f, FColor::Emerald, FString::Printf(TEXT("Current %s skill is on cooldown!!"), ((*name.ToString()))));
								GEngine->AddOnScreenDebugMessage(-1, 10.0f, FColor::Emerald, FString::Printf(TEXT("Current %s skill is on cooldown!!"), ((*name.ToString()))));
								cooldown = row->isOnCD;
								break;
							}
							else if (row->isOnCD == false)
							{
								cooldown = row->isOnCD;
								row->isOnCD = true;
								//GEngine->AddOnScreenDebugMessage(-1, 10.0f, FColor::Emerald, FString::Printf(TEXT("Current key is %s"), ((*row->keys.ToString()))));
								if (row->SkillMoveset != nullptr)
								{
									if (this->IsLocallyControlled())
									{
										AttackSection = "NormalAttack01";

										//play the animation that visible to all clients
										ServerExecuteAction(*row, CurrentSection, AttackSection, false);

										//setting up for cooldown properties
										FTimerHandle handle;
										FTimerDelegate TimerDelegate;

										//set the row boolean to false after finish cooldown timer
										TimerDelegate.BindLambda([row, this]()
										{
											UE_LOG(LogTemp, Warning, TEXT("DELAY BEFORE SETTING UP COOLDOWN TO FALSE"));
											row->isOnCD = false;

											//GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Blue, FString::Printf(TEXT("row->isOnCD: %s"), row->isOnCD ? TEXT("true") : TEXT("false")));
										});

										//start cooldown the skill
										this->GetWorldTimerManager().SetTimer(handle, TimerDelegate, row->CDDuration, false);
										CooldownVal = row->CDDuration;
										break;
									}
								}
								break;
							}
						}

						/**   current skill has combo */
						else if (row->UseSection)
						{
							//GEngine->AddOnScreenDebugMessage(-1, 10.0f, FColor::Emerald, FString::Printf(TEXT("Current key is %s"), ((*row->keys.ToString()))));
							if (row->SkillMoveset != nullptr)
							{
								TargetHead = row->TargetIsHead;
								if (this->IsLocallyControlled())
								{
									DetectNearestTarget(EResult::Section, *row);
								}
								/*AttackCombo(*row);*/
								break;

							}
						}
					}
				}
			}
		}
	}
}

void ABattleMobaCharacter::AttackCombo(FActionSkill SelectedRow)
{
	/**		checks if the mesh has a skeletal mesh*/
	if (GetMesh()->SkeletalMesh != nullptr)
	{
		if (AnimInsta != nullptr)
		{
			/**		Continues combo section of the montage if there is montage playing */
		//if (AnimInsta->Montage_IsPlaying(FastComboMoveset)||(AnimInsta->Montage_IsPlaying(StrongkComboMoveset)))
			if (AnimInsta->IsAnyMontagePlaying())
			{
				/**		Checks whether the current montage section contains "Combo" substring */
				CurrentSection = AnimInsta->Montage_GetCurrentSection(AnimInsta->GetCurrentActiveMontage());

				/**		Cue "ServerExecuteAction" after current section is finished playing */

				if (SelectedRow.Section == 3)
				{
					if (UKismetStringLibrary::Contains(CurrentSection.ToString(), TEXT("Combo"), false, false))
					{
						/**		Checks if current combo section contains "01" substring*/
						if (UKismetStringLibrary::Contains(CurrentSection.ToString(), TEXT("01"), false, false))
						{
							FString NextSection = UKismetStringLibrary::Concat_StrStr(TEXT("NormalAttack"), TEXT("02"));
							AttackSection = FName(*NextSection);

							if (this->IsLocallyControlled())
							{
								/**		Change next attack to combo montage section*/
								ServerExecuteAction(SelectedRow, CurrentSection, AttackSection, false);
							}
						}

						else if (UKismetStringLibrary::Contains(CurrentSection.ToString(), TEXT("02"), false, false))
						{
							FString NextSection = UKismetStringLibrary::Concat_StrStr(TEXT("NormalAttack"), TEXT("03"));
							AttackSection = FName(*NextSection);

							if (this->IsLocallyControlled())
							{
								/**		Change next attack to combo montage section*/
								ServerExecuteAction(SelectedRow, CurrentSection, AttackSection, false);
							}
						}

						//else if (UKismetStringLibrary::Contains(CurrentSection.ToString(), TEXT("03"), false, false))
						//{
						//	FString NextSection = UKismetStringLibrary::Concat_StrStr(TEXT("NormalAttack"), TEXT("01"));
						//	AttackSection = FName(*NextSection);
						//	
						//	if (this->IsLocallyControlled())
						//	{
						//		/**		Change next attack to combo montage section*/
						//		ServerExecuteAction(SelectedRow, CurrentSection, AttackSection, false);
						//	}
						//}
					}
				}

				else if (SelectedRow.Section == 2)
				{
					if (UKismetStringLibrary::Contains(CurrentSection.ToString(), TEXT("Combo"), false, false))
					{
						/**		Checks if current combo section contains "01" substring*/
						if (UKismetStringLibrary::Contains(CurrentSection.ToString(), TEXT("01"), false, false))
						{
							FString NextSection = UKismetStringLibrary::Concat_StrStr(TEXT("NormalAttack"), TEXT("02"));
							AttackSection = FName(*NextSection);

							if (this->IsLocallyControlled())
							{
								/**		Change next attack to combo montage section*/
								ServerExecuteAction(SelectedRow, CurrentSection, AttackSection, false);
							}
						}

						//else if (UKismetStringLibrary::Contains(CurrentSection.ToString(), TEXT("02"), false, false))
						//{
						//	FString NextSection = UKismetStringLibrary::Concat_StrStr(TEXT("NormalAttack"), TEXT("01"));
						//	AttackSection = FName(*NextSection);

						//	if (this->IsLocallyControlled())
						//	{
						//		/**		Change next attack to combo montage section*/
						//		ServerExecuteAction(SelectedRow, CurrentSection, AttackSection, false);
						//	}
						//	
						//}

					}
				}
			}
			/**		Plays the first section of the montage*/
			else
			{
				bAttacking = true;
				FTimerHandle Timer;
				FTimerDelegate TimerDelegate;
				AttackSection = "NormalAttack01";
				CurrentSection = "NormalAttack01";

				if (IsLocallyControlled())
				{
					ServerExecuteAction(SelectedRow, CurrentSection, AttackSection, false);
				}

				TimerDelegate.BindLambda([this]()
				{
					bAttacking = false;
					//GEngine->AddOnScreenDebugMessage(-1, 10.0f, FColor::Emerald, FString::Printf(TEXT(" bCombo Attack resets to false")));
				});

				/**		Reset boolean after section ends*/
				this->GetWorldTimerManager().SetTimer(Timer, TimerDelegate, AttackSectionLength, false);
			}
		}
		
	}
}

bool ABattleMobaCharacter::ServerCounterAttack_Validate(ABattleMobaCharacter* hitActor)
{
	return true;
}

void ABattleMobaCharacter::ServerCounterAttack_Implementation(ABattleMobaCharacter* hitActor)
{
	MulticastCounterAttack(hitActor);
}

bool ABattleMobaCharacter::MulticastCounterAttack_Validate(ABattleMobaCharacter* hitActor)
{
	return true;
}

void ABattleMobaCharacter::MulticastCounterAttack_Implementation(ABattleMobaCharacter* hitActor)
{
	FName CounterCurrentSection = hitActor->AnimInsta->Montage_GetCurrentSection(hitActor->CounterMoveset);

	/**		If current montage section is CheckInput. go to next available section*/
	if (UKismetStringLibrary::Contains(CounterCurrentSection.ToString(), TEXT("CheckInput"), false, false))
	{
		FTimerHandle Delay;
		FString NextSection = "CounterAttack01";
		hitActor->AttackSection = FName(*NextSection);
		
		hitActor->PlayAnimMontage(hitActor->CounterMoveset, 1.0f, hitActor->AttackSection);
		
		//setting up for translate properties
		FTimerHandle handle;
		FTimerDelegate TimerDelegate;

		//launch player forward after 0.5s
		TimerDelegate.BindLambda([this, hitActor]()
		{
			UE_LOG(LogTemp, Warning, TEXT("DELAY BEFORE TRANSLATE CHARACTER FORWARD"));

			FVector dashVector = FVector(hitActor->GetCapsuleComponent()->GetForwardVector().X*1000.0f, hitActor->GetCapsuleComponent()->GetForwardVector().Y*1000.0f, hitActor->GetCapsuleComponent()->GetForwardVector().Z);

			hitActor->LaunchCharacter(dashVector, true, true);
		});
		/**		cooldown to execute lines inside TimerDelegate*/
		hitActor->GetWorldTimerManager().SetTimer(handle, TimerDelegate, 0.5f, false);

	}
}

bool ABattleMobaCharacter::RespawnCharacter_Validate()
{
	return true;
}

void ABattleMobaCharacter::RespawnCharacter_Implementation()
{
	ABattleMobaPC* PC = Cast<ABattleMobaPC>(UGameplayStatics::GetPlayerController(this, 0));
	if (PC)
	{
		//Set current input to interact with UI in spectator mode
		if (PC->IsLocalPlayerController() && PC->GetNetMode() != ENetMode::NM_DedicatedServer)
		{
			PC->bShowMouseCursor = true;
			PC->SetInputMode(FInputModeGameAndUI());
		}
		ABattleMobaPlayerState* PS = Cast<ABattleMobaPlayerState>(PC->PlayerState);
		if (PS)
		{
			PS->RespawnTimeCounter -= 1;
			PS->DisplayRespawnTime();
			PC->RespawnPawn(PS->SpawnTransform);
			PC->UnPossess();
		}
	}
}

void ABattleMobaCharacter::EnableMovementMode()
{
	if (GetMesh()->SkeletalMesh != nullptr)
	{
		if (this->AnimInsta)
		{
			this->AnimInsta->CanMove = true;
		}
	}
}

bool ABattleMobaCharacter::DetectNearestTarget_Validate(EResult Type, FActionSkill SelectedRow)
{
	return true;
}

void ABattleMobaCharacter::DetectNearestTarget_Implementation(EResult Type, FActionSkill SelectedRow)
{
	//		create tarray for hit results
	TArray<FHitResult> hitResults;

	//		start and end locations
	FVector Start = this->GetActorLocation();
	FVector End = FVector(this->GetActorLocation().X + 0.1f, this->GetActorLocation().Y + 0.1f, this->GetActorLocation().Z + 0.1f);

	//		create a collision sphere with radius of RotateRadius
	FCollisionShape SphereCol = FCollisionShape::MakeSphere(RotateRadius);

	FCollisionQueryParams TraceParams;
	TraceParams.AddIgnoredActor(this);

	float Distance1;
	float Distance2;

	//TArray< TEnumAsByte<EObjectTypeQuery> > ObjectTypes;
	//ObjectTypes.Add(UEngineTypes::ConvertToObjectType(ECC_PhysicsBody));

	//		check if something got hit in the sweep
	bool isHit = GetWorld()->SweepMultiByChannel(hitResults, Start, End, FQuat::Identity, ECC_PhysicsBody, SphereCol, TraceParams);

	//		draw a purple collision sphere for 0.5 seconds
	DrawDebugSphere(GetWorld(), GetActorLocation(), SphereCol.GetSphereRadius(), 8, FColor::Purple, false, 0.5);

	//bool isHit = UKismetSystemLibrary::SphereTraceMultiForObjects(GetWorld(), Start, End, RotateRadius, ObjectTypes, true, IgnoreActors, EDrawDebugTrace::ForDuration, hitResults, true);
	if (isHit)
	{
		for (auto& Hit : hitResults)
		{
			ABattleMobaCharacter* pc = Cast<ABattleMobaCharacter>(Hit.Actor);
			ADestructibleTower* tower = Cast<ADestructibleTower>(Hit.Actor);

			if (pc != nullptr)
			{
				if (pc->InRagdoll == false && pc->TeamName != this->TeamName)
				{
					//		check current closestActor is still the closest or change to new one
					if (IsValid(closestActor))
					{
						Distance1 = closestActor->GetDistanceTo(this);
						Distance2 = Hit.Actor->GetDistanceTo(this);

						if (Distance1 < Distance2)
						{

						}

						//		if distance is larger or same set new closest actor
						else
						{
							closestActor = Cast<AActor>(Hit.Actor);
							//GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, FString::Printf(TEXT("Closest Actor: %s"), *closestActor->GetName()));

						}
					}

					else
					{
						closestActor = Cast<AActor>(Hit.Actor);
						//GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, FString::Printf(TEXT("Closest Actor: %s"), *closestActor->GetName()));
					}
				}
			}

			else if (tower != nullptr && pc == nullptr)
			{
				if (tower->TeamName != this->TeamName)
				{
					//		prioritise player, only then set closest actor to tower
					if (IsValid(closestActor) == false)
					{
						closestActor = Cast<AActor>(Hit.Actor);
					}
					
				}
			}
			//GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, FString::Printf(TEXT("Hit Result: %s"), *Hit.Actor->GetName()));
		}
		RotateNearestTarget(closestActor, Type, SelectedRow);
	}
}

bool ABattleMobaCharacter::RotateNearestTarget_Validate(AActor* Target, EResult Type, FActionSkill SelectedRow)
{
	return true;
}

void ABattleMobaCharacter::RotateNearestTarget_Implementation(AActor* Target, EResult Type, FActionSkill SelectedRow)
{
	if (IsValid(Target))
	{
		FLatentActionInfo LatentInfo = FLatentActionInfo();
		LatentInfo.CallbackTarget = this;

		FRotator LookRotation = UKismetMathLibrary::FindLookAtRotation(this->GetCapsuleComponent()->GetComponentLocation(), Target->GetActorLocation());
		FRotator RotateTo = FRotator(this->GetCapsuleComponent()->GetComponentRotation().Pitch, LookRotation.Yaw, this->GetCapsuleComponent()->GetComponentRotation().Roll);
		//FMath::RInterpTo(this->GetCapsuleComponent()->GetComponentRotation(), RotateTo, this->GetWorld()->GetDeltaSeconds(), 100.0f);

		//Check for Character to move the component target, else just rotate
		ABattleMobaCharacter* characterActor = Cast<ABattleMobaCharacter>(Target);

		//If the distance between two character is outside range
		if (this->GetDistanceTo(Target) > RotateRadius && characterActor)
		{
			//Get Vector from player minus target
			FVector FromOriginToTarget = this->GetActorLocation() - Target->GetActorLocation();

			//To avoid overlapping actors
			//Multiply by Radius and divided by distance
			FromOriginToTarget *= RotateRadius / this->GetDistanceTo(Target);

			GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, FString::Printf(TEXT("Hit Speed: %f"), FromOriginToTarget.Size()));

			UBattleMobaAnimInstance* inst = Cast<UBattleMobaAnimInstance>(this->GetMesh()->GetAnimInstance());
			inst->Speed = FromOriginToTarget.Size();
			inst->bMoving = true;

			//rotate and move the component towards target
			UKismetSystemLibrary::MoveComponentTo(this->GetCapsuleComponent(), Target->GetActorLocation() + FromOriginToTarget, RotateTo, true, true, 0.1f, true, EMoveComponentAction::Type::Move, LatentInfo);

			//setting up delay properties
			FTimerHandle handle;
			FTimerDelegate TimerDelegate;

			TimerDelegate.BindLambda([this, inst, Type, SelectedRow]()
			{
				inst->Speed = 0.0f;
				inst->bMoving = false;

				//execute action skill
				if (this->IsLocallyControlled())
				{
					if (Type == EResult::Cooldown)
					{
						ServerExecuteAction(SelectedRow, CurrentSection, AttackSection, true);
					}
					else if (Type == EResult::Section)
					{
						AttackCombo(SelectedRow);
					}
				}
			});
			/*Start delay to reset speed*/
			this->GetWorldTimerManager().SetTimer(handle, TimerDelegate, 0.1f, false);

		}
		else
		{
			//rotate the component towards target
			UKismetSystemLibrary::MoveComponentTo(this->GetCapsuleComponent(), this->GetCapsuleComponent()->GetComponentLocation(), RotateTo, true, true, 0.1f, true, EMoveComponentAction::Type::Move, LatentInfo);

			//execute action skill
			if (this->IsLocallyControlled())
			{
				if (Type == EResult::Cooldown)
				{
					ServerExecuteAction(SelectedRow, CurrentSection, AttackSection, true);
				}
				else if (Type == EResult::Section)
				{
					AttackCombo(SelectedRow);
				}
			}
		}
		closestActor = nullptr;
	}
	else
	{
		//Execute when closestactor is invalid
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, FString::Printf(TEXT("INVALID")));

		//execute action skill
		if (this->IsLocallyControlled())
		{
			if (Type == EResult::Cooldown)
			{
				ServerExecuteAction(SelectedRow, CurrentSection, AttackSection, true);
			}
			else if (Type == EResult::Section)
			{
				AttackCombo(SelectedRow);
			}
		}
	}
}


void ABattleMobaCharacter::SafeZone(ABMobaTriggerCapsule* TriggerZone)
{
	UUserWidget* HPWidget = Cast<UUserWidget>(TriggerZone->W_Val->GetUserWidgetObject());
	if (HPWidget)
	{
		const FName hpbar = FName(TEXT("PBar"));
		UProgressBar* PBar = (UProgressBar*)(HPWidget->WidgetTree->FindWidget(hpbar));

		const FName hptext = FName(TEXT("ValText"));
		UTextBlock* HealthText = (UTextBlock*)(HPWidget->WidgetTree->FindWidget(hptext));

		if (PBar)
		{
			//FSlateBrush newBrush;
			if (this->IsLocallyControlled())
			{
				//Change to progressbar color to blue
				if (PBar->Percent <= 0.0f)
				{
					PBar->SetFillColorAndOpacity(FLinearColor(0.0f, 0.5f, 1.0f));
					if (HealthText)
					{
						HealthText->SetColorAndOpacity(FLinearColor(0.0f, 0.5f, 1.0f));
					}
				}
				SafeZoneServer(TriggerZone);
			}
			else
			{
				//GEngine->AddOnScreenDebugMessage(-1, 10.0f, FColor::Red, FString::Printf(TEXT("Current Team is %s"), ((*Team.ToString()))));
				//GEngine->AddOnScreenDebugMessage(-1, 10.0f, FColor::Red, FString::Printf(TEXT("Team is %s"), ((*this->TeamName.ToString()))));
				if (PBar->Percent <= 0.0f)
				{
					if (this->TeamName == TriggerZone->TeamName)
					{
						//Change progressbar color to red
						PBar->SetFillColorAndOpacity(FLinearColor(1.0f, 0.0f, 0.0f));
						if (HealthText)
						{
							HealthText->SetColorAndOpacity(FLinearColor(1.0f, 0.0f, 0.0f));
						}
					}
					else
					{
						//GEngine->AddOnScreenDebugMessage(-1, 10.0f, FColor::Emerald, FString::Printf(TEXT("Team is %s"), ((*this->TeamName.ToString()))));
						//Change progressbar color to red
						PBar->SetFillColorAndOpacity(FLinearColor(0.0f, 0.5f, 1.0f));
						if (HealthText)
						{
							HealthText->SetColorAndOpacity(FLinearColor(0.0f, 0.5f, 1.0f));
						}
						//newBrush.TintColor = FLinearColor(1.0f, 0.0f, 0.0f);
					}
				}
			}
			//PBar->WidgetStyle.SetBackgroundImage(newBrush);
		}
	}
	//if (IsLocallyControlled())
	//{
	//	//Run server safezone
	//	SafeZoneServer(TriggerZone);
	//}
}

bool ABattleMobaCharacter::SafeZoneServer_Validate(ABMobaTriggerCapsule* TriggerZone)
{
	return true;
}

void ABattleMobaCharacter::SafeZoneServer_Implementation(ABMobaTriggerCapsule* TriggerZone)
{
	//Check if no server timer is running, start the timer, else stop the timer
	if (GetWorld()->GetTimerManager().IsTimerActive(TriggerZone->FlagTimer) == false)
	{
		FTimerDelegate FunctionsNames = FTimerDelegate::CreateUObject(this, &ABattleMobaCharacter::SafeZoneMulticast, TriggerZone);
		GetWorld()->GetTimerManager().SetTimer(TriggerZone->FlagTimer, FunctionsNames, 1.0f, true);
		return;
	}
	else
	{
		GetWorld()->GetTimerManager().ClearTimer(TriggerZone->FlagTimer);
		//SafeZoneMulticast(TriggerZone);
	}
}

bool ABattleMobaCharacter::SafeZoneMulticast_Validate(ABMobaTriggerCapsule* TriggerZone)
{
	return true;
}

void ABattleMobaCharacter::SafeZoneMulticast_Implementation(ABMobaTriggerCapsule* TriggerZone)
{
	//GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Orange, FString::Printf(TEXT("SAFE ZONE??????????")));
	
	TriggerZone->val = TriggerZone->val + 1;
	TriggerZone->OnRep_Val();
}

void ABattleMobaCharacter::ControlFlagMode(ABattleMobaCTF* cf)
{
	if (cf->isCompleted == false)
	{
		ABattleMobaGameState* thisGS = Cast<ABattleMobaGameState>(UGameplayStatics::GetGameState(this));
		if (thisGS)
		{
			thisGS->SetTowerWidgetColors(cf);
		}
	}

	if (this->IsLocallyControlled())
	{
		//Run server Control Flag
		ControlFlagServer(cf);
	}
}

bool ABattleMobaCharacter::ControlFlagServer_Validate(ABattleMobaCTF * cf)
{
	return true;
}

void ABattleMobaCharacter::ControlFlagServer_Implementation(ABattleMobaCTF* cf)
{
	if (cf->RadiantControl > 0 && cf->DireControl == 0)
	{
		CTFteam = "Radiant";
		ControlFlagMulticast(cf, CTFteam);
	}

	else if (cf->DireControl > 0 && cf->RadiantControl == 0)
	{
		CTFteam = "Dire";
		ControlFlagMulticast(cf, CTFteam);
	}

	else
	{
		CTFteam = "";
		ControlFlagMulticast(cf, CTFteam);

	}

	cf->RadiantControl = 0;
	cf->DireControl = 0;
	//GEngine->AddOnScreenDebugMessage(-1, 10.0f, FColor::Emerald, FString::Printf(TEXT("Current CTF Team is %s"), ((*CTFteam.ToString()))));
}

bool ABattleMobaCharacter::ControlFlagMulticast_Validate(ABattleMobaCTF* cf, FName Team)
{
	return true;
}

void ABattleMobaCharacter::ControlFlagMulticast_Implementation(ABattleMobaCTF* cf, FName Team)
{
	if (Team == "Radiant")
	{
		//	Decrease the valDire if exists first before increasing valRadiant
		if (cf->valDire <= 0.0f)
		{
			cf->valDire = 0.0f;

			if (cf->valRadiant < 100.0f)
			{
				cf->valRadiant = cf->valRadiant + 1;
				cf->isCompleted = false;
			}

			else
			{
				cf->valRadiant = 100.0f;
				cf->ControllerTeam = "Radiant";
				cf->isCompleted = true;	
			}
		}

		else
		{
			cf->valDire = cf->valDire - 1;
			cf->isCompleted = false;
		}

		cf->OnRep_Val();
	}

	else if (Team == "Dire")
	{
		if (cf->valRadiant <= 0.0f)
		{
			cf->valRadiant = 0.0f;

			if (cf->valDire < 100.0f)
			{
				cf->valDire = cf->valDire + 1;
				cf->isCompleted = false;
			}
			
			else
			{
				cf->valDire = 100.0f;
				cf->ControllerTeam = "Dire";
				cf->isCompleted = true;
			}
		}

		else
		{
			cf->valRadiant = cf->valRadiant - 1;
			cf->isCompleted = false;
		}

		cf->OnRep_Val();
	}
}

bool ABattleMobaCharacter::SetupStats_Validate()
{
	return true;
}

void ABattleMobaCharacter::SetupStats_Implementation()
{
	UUserWidget* HPWidget = Cast<UUserWidget>(W_DamageOutput->GetUserWidgetObject());
	if (HPWidget)
	{
		//GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Blue, FString::Printf(TEXT("Player %s with %s Widget"), *GetDebugName(this), *HPWidget->GetFName().ToString()));
		const FName hptext = FName(TEXT("HealthText"));
		UTextBlock* HealthText = (UTextBlock*)(HPWidget->WidgetTree->FindWidget(hptext));

		const FName hpbar = FName(TEXT("HPBar"));
		UProgressBar* HealthBar = (UProgressBar*)(HPWidget->WidgetTree->FindWidget(hpbar));

		if (HealthText)
		{
			FString TheFloatStr = FString::SanitizeFloat(this->Health);

			HealthText->SetText(FText::FromString(TheFloatStr));
			HealthBar->SetPercent(FMath::Clamp(this->Health / 100.0f, 0.0f, 1.0f));
		}
	}
}

bool ABattleMobaCharacter::MulticastExecuteAction_Validate(FActionSkill SelectedRow, FName ActiveSection, FName MontageSection, bool bSpecialAttack)
{
	return true;
}

void ABattleMobaCharacter::MulticastExecuteAction_Implementation(FActionSkill SelectedRow, FName ActiveSection, FName MontageSection, bool bSpecialAttack)
{
	/**		Checks SkeletalMesh exists / AnimInst Exists / Player is Stunned or still executing a skill */
	if (this->GetMesh()->SkeletalMesh != nullptr)
	{
		if (this->AnimInsta != nullptr && this->IsStunned == false)
		{
			//GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Blue, FString::Printf(TEXT("row->isOnCD: %s"), SelectedRow.isOnCD ? TEXT("true") : TEXT("false")));

			/**		Disable movement on Action Skill*/
			this->AnimInsta->CanMove = false;
			this->OnSpecialAttack = bSpecialAttack;

			if (bSpecialAttack == true)
			{
				/*if (test == true)
				{
					RotateToTargetSetup();
				}*/

				//if current montage consumes cooldown properties
				if (SelectedRow.IsUsingCD)
				{
					/**		set the counter moveset to skillmoveset*/
					this->CounterMoveset = SelectedRow.SkillMoveset;


					///GEngine->AddOnScreenDebugMessage(-1, 10.0f, FColor::Emerald, FString::Printf(TEXT("Play montage: %s"), *SelectedRow.SkillMoveset->GetName()));
					//GEngine->AddOnScreenDebugMessage(-1, 10.0f, FColor::Emerald, FString::Printf(TEXT("ISUSINGCD")));

					PlayAnimMontage(SelectedRow.SkillMoveset, 1.0f, MontageSection);
				}
			}

			else
			{
				if (SelectedRow.UseTranslate)
				{
					FTimerHandle Delay;

					if (SelectedRow.IsUsingCD)
					{
						//GEngine->AddOnScreenDebugMessage(-1, 10.0f, FColor::Emerald, FString::Printf(TEXT("Play montage: %s"), *SelectedRow.SkillMoveset->GetName()));

						float montageTimer = this->GetMesh()->GetAnimInstance()->Montage_Play(SelectedRow.SkillMoveset, 1.0f, EMontagePlayReturnType::MontageLength, 0.0f, true);

						////setting up for translate properties
						//FTimerHandle handle;
						//FTimerDelegate TimerDelegate;

						////launch player forwa rd after 0.218f
						//TimerDelegate.BindLambda([this, SelectedRow]()
						//{
						//	UE_LOG(LogTemp, Warning, TEXT("DELAY BEFORE TRANSLATE CHARACTER FORWARD"));

						//	FVector dashVector = FVector(this->GetCapsuleComponent()->GetForwardVector().X*SelectedRow.TranslateDist, this->GetCapsuleComponent()->GetForwardVector().Y*SelectedRow.TranslateDist, this->GetCapsuleComponent()->GetForwardVector().Z);

						//	this->LaunchCharacter(dashVector, true, true);
						//});
						///**		cooldown to execute lines inside TimerDelegate*/
						//this->GetWorldTimerManager().SetTimer(handle, TimerDelegate, 0.6f, false);

						///**		Enable movement back after montage finished playing*/
						//this->GetWorld()->GetTimerManager().SetTimer(Delay, this, &ABattleMobaCharacter::EnableMovementMode, montageTimer, false);
					}
				}

				else if (SelectedRow.UseSection)
				{
					if (this->IsLocallyControlled())
					{
						/*FoundActors.Empty();
						RotateToTargetSetup();*/
						/*if (test == true)
						{
							RotateToTargetSetup();
						}*/
					}
					
					
					if (AnimInsta->Montage_IsPlaying(SelectedRow.SkillMoveset))
					{
						if (ActiveSection != MontageSection)
						{
							AnimInsta->Montage_SetNextSection(ActiveSection, MontageSection, SelectedRow.SkillMoveset);
						}
					}

					else
					{
						AttackSectionLength = this->GetMesh()->GetAnimInstance()->Montage_Play(SelectedRow.SkillMoveset, 1.0f, EMontagePlayReturnType::MontageLength, 0.0f, true);
					}
										
					//PlayAnimMontage(SelectedRow.SkillMoveset, 1.0f, MontageSection);
				}

				else
				{
					//GEngine->AddOnScreenDebugMessage(-1, 10.0f, FColor::Emerald, FString::Printf(TEXT("STATEMENT ELSE")));
				}

			}

			this->MinDamage = SelectedRow.MinDamage;
			this->MaxDamage = SelectedRow.MaxDamage;
			this->BaseDamage = float(FMath::RandRange(this->MinDamage, this->MaxDamage));
			this->HitReactionMoveset = SelectedRow.HitMoveset;
			this->FrontHitMoveset = SelectedRow.FrontHitMoveset;
			this->BackHitMoveset = SelectedRow.BackHitMoveset;
			this->LeftHitMoveset = SelectedRow.LeftHitMoveset;
			this->RightHitMoveset = SelectedRow.RightHitMoveset;
			this->HitEffect = SelectedRow.HitImpact;
		}	
	}
}

void ABattleMobaCharacter::OnCombatColl(UCapsuleComponent* CombatColl)
{
	CombatColl->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	CombatColl->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Ignore);
}

void ABattleMobaCharacter::OffCombatColl(UCapsuleComponent * CombatColl)
{
	CombatColl->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	DoOnce = false;
	TargetHead = false;
}

bool ABattleMobaCharacter::CallAttackTrace_Validate(bool isStart, int switcher)
{
	return true;
}

void ABattleMobaCharacter::CallAttackTrace_Implementation(bool isStart, int switcher)
{
	FTimerDelegate AttackTD;

	if (isStart)
	{
		AttackTD.BindUFunction(this, FName("AttackTrace"), switcher);
		GetWorld()->GetTimerManager().SetTimer(AttackTimer, AttackTD, 0.01f, true);
		return;

	}
	else
	{
		GetWorld()->GetTimerManager().ClearTimer(AttackTimer);
		ArrDamagedEnemy.Empty();
	}
}


bool ABattleMobaCharacter::AttackTrace_Validate(bool traceStart, int activeAttack)
{
	return true;
}

void ABattleMobaCharacter::AttackTrace_Implementation(bool traceStart, int activeAttack)
{
	GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::Red, FString::Printf(TEXT("Start Tracing? %s"), traceStart ? TEXT("True") : TEXT("False")));

	if (traceStart)
	{
		FHitResult hitResult;

		FVector startTrace;
		FVector endTrace;

		TArray< TEnumAsByte<EObjectTypeQuery> > ObjectTypes;
		ObjectTypes.Add(UEngineTypes::ConvertToObjectType(ECC_PhysicsBody));

		ActiveColliders.Empty();

		if (activeAttack == 1)
		{
			ActiveColliders.Add(LPC1);
			ActiveColliders.Add(LPC2);
			ActiveColliders.Add(LPC3);
			ActiveColliders.Add(LPC4);
			ActiveColliders.Add(LPC5);
			ActiveColliders.Add(LPC6);
		}

		else if (activeAttack == 2)
		{
			ActiveColliders.Add(RPC1);
			ActiveColliders.Add(RPC2);
			ActiveColliders.Add(RPC3);
			ActiveColliders.Add(RPC4);
			ActiveColliders.Add(RPC5);
			ActiveColliders.Add(RPC6);
		}

		else if (activeAttack == 3)
		{
			ActiveColliders.Add(LKC1);
			ActiveColliders.Add(LKC2);
			ActiveColliders.Add(LKC3);
			ActiveColliders.Add(LKC4);
			ActiveColliders.Add(LKC5);
			ActiveColliders.Add(LKC6);
		}

		else if (activeAttack == 4)
		{
			ActiveColliders.Add(RKC1);
			ActiveColliders.Add(RKC2);
			ActiveColliders.Add(RKC3);
			ActiveColliders.Add(RKC4);
			ActiveColliders.Add(RKC5);
			ActiveColliders.Add(RKC6);
		}

		for (auto& attackCol : ActiveColliders)
		{
			startTrace = attackCol->GetComponentLocation();
			endTrace = startTrace + (GetActorForwardVector() * TraceDistance);

			//bool bHit = GetWorld()->LineTraceSingleByObjectType(hitResult, startTrace, endTrace, FCollisionObjectQueryParams(ECC_TO_BITFIELD(ECC_Attack)), FCollisionQueryParams(TEXT("IKTrace"), false, this));
			bool bHit = UKismetSystemLibrary::LineTraceSingleForObjects(GetWorld(), startTrace, endTrace, ObjectTypes, true, IgnoreActors, EDrawDebugTrace::ForDuration, hitResult, true, FColor::Red, FColor::Green, -1.0f);

			GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::Red, FString::Printf(TEXT("Blocking hit is %s"), bHit ? TEXT("True") : TEXT("False")));
			//GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Green, FString::Printf(TEXT("You are hitting: %s"), *UKismetSystemLibrary::GetDisplayName(hitResults.Actor)));
			if (bHit)
			{
				HitResult(hitResult);
			}
		}
	}

	else
	{
		ArrDamagedEnemy.Empty();
	}
}

bool ABattleMobaCharacter::HitResult_Validate(FHitResult hit)
{
	return true;
}

void ABattleMobaCharacter::HitResult_Implementation(FHitResult hit)
{
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), ABattleMobaCharacter::StaticClass(), HitActorsFound);

	ABattleMobaCharacter* DamagedEnemy;

	int ArrayLength = HitActorsFound.Num();

	for (uint8 i = 0; i < ArrayLength; ++i)
	{
		DamagedEnemy = Cast<ABattleMobaCharacter>(HitActorsFound[i]);

		if (DamagedEnemy->TeamName != this->TeamName && (DamagedEnemy == hit.Actor) && !(ArrDamagedEnemy.Contains(DamagedEnemy)))
		{
			ArrDamagedEnemy.Add(DamagedEnemy);
			DoDamage(DamagedEnemy);
		}
	}
}

bool ABattleMobaCharacter::DoDamage_Validate(AActor* HitActor)
{
	return true;
}

void ABattleMobaCharacter::DoDamage_Implementation(AActor* HitActor)
{
	if (this != HitActor)
	{
		/**		Calculate Damage Dealt by Enemy and set precision to tenth */
		this->ActualDamage = (this->BaseDamage * this->BuffDamage) * (100 / (100 + ((Defence * BuffDefence) * ((1 - ReducedDefence) * 0.84))));
		this->ActualDamage = FMath::RoundToInt(ActualDamage);

		/**		Apply Damage */
		//GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Magenta, FString::Printf(TEXT("Damage Applied: %f"), this->ActualDamage));
		UE_LOG(LogTemp, Warning, TEXT("Damage Applied: %f"), this->ActualDamage);
		this->ActualDamage = UGameplayStatics::ApplyDamage(HitActor, this->ActualDamage, nullptr, this, nullptr);
	}
}

bool ABattleMobaCharacter::FireTrace_Validate(UBoxComponent* Col1, UBoxComponent* Col2, UBoxComponent* Col3, UBoxComponent* Col4, UBoxComponent* Col5, UBoxComponent* Col6)
{
	return true;
}

void ABattleMobaCharacter::FireTrace_Implementation(UBoxComponent* Col1, UBoxComponent* Col2, UBoxComponent* Col3, UBoxComponent* Col4, UBoxComponent* Col5, UBoxComponent* Col6)
{
	if (this->GetMesh()->SkeletalMesh != nullptr)
	{
		if (this->AnimInsta != nullptr)
		{
			GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::Magenta, FString::Printf(TEXT("Can Attack = %s"), AnimInsta->canAttack ? TEXT("True") : TEXT("False")));
			if (AnimInsta->canAttack == true)
			{
				GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::Yellow, FString::Printf(TEXT("Apply Hit Trace = %s"), bApplyHitTrace ? TEXT("True") : TEXT("False")));
				//		stop the hit happening again
				if (bApplyHitTrace == true)
				{
					FHitResult hitRes1;
					bool bHit1 = GetWorld()->LineTraceSingleByChannel(hitRes1, Col1->GetComponentLocation(), Col1->GetComponentLocation() + ( GetActorForwardVector() * TraceDistance), ECC_PhysicsBody, AttackTraceParams);

					if (bHit1)
					{
						bApplyHitTrace = false;
						//		pass hit results to further give conditions on damage
						ABattleMobaCharacter* hitChar = Cast<ABattleMobaCharacter>(hitRes1.Actor);
						ADestructibleTower* hitTower = Cast<ADestructibleTower>(hitRes1.Actor);

						if (hitChar && hitChar->InRagdoll == false && hitChar->TeamName != this->TeamName)
						{
							DoDamage(hitChar);
							bApplyHitTrace = true;
						}
					}

					else
					{
						FHitResult hitRes2;
						bool bHit2 = GetWorld()->LineTraceSingleByChannel(hitRes2, Col2->GetComponentLocation(), Col2->GetComponentLocation() + (GetActorForwardVector() * TraceDistance), ECC_PhysicsBody, AttackTraceParams);

						if (bHit2)
						{
							bApplyHitTrace = false;
							ABattleMobaCharacter* hitChar = Cast<ABattleMobaCharacter>(hitRes2.Actor);

							if (hitChar && hitChar->InRagdoll == false && hitChar->TeamName != this->TeamName)
							{
								DoDamage(hitChar);
								bApplyHitTrace = true;
							}
						}

						else
						{
							FHitResult hitRes3;
							bool bHit3 = GetWorld()->LineTraceSingleByChannel(hitRes3, Col3->GetComponentLocation(), Col3->GetComponentLocation() + (GetActorForwardVector() * TraceDistance), ECC_PhysicsBody, AttackTraceParams);

							if (bHit3)
							{
								bApplyHitTrace = false;
								ABattleMobaCharacter* hitChar = Cast<ABattleMobaCharacter>(hitRes3.Actor);

								if (hitChar && hitChar->InRagdoll == false && hitChar->TeamName != this->TeamName)
								{
									DoDamage(hitChar);
									bApplyHitTrace = true;
								}
							}

							else
							{
								FHitResult hitRes4;
								bool bHit4 = GetWorld()->LineTraceSingleByChannel(hitRes4, Col4->GetComponentLocation(), Col4->GetComponentLocation() + (GetActorForwardVector() * TraceDistance), ECC_PhysicsBody, AttackTraceParams);

								if (bHit4)
								{
									bApplyHitTrace = false;
									ABattleMobaCharacter* hitChar = Cast<ABattleMobaCharacter>(hitRes4.Actor);

									if (hitChar && hitChar->InRagdoll == false && hitChar->TeamName != this->TeamName)
									{
										DoDamage(hitChar);
										bApplyHitTrace = true;
									}
								}

								else
								{
									FHitResult hitRes5;
									bool bHit5 = GetWorld()->LineTraceSingleByChannel(hitRes5, Col5->GetComponentLocation(), Col5->GetComponentLocation() + (GetActorForwardVector() * TraceDistance), ECC_PhysicsBody, AttackTraceParams);

									if (bHit5)
									{
										bApplyHitTrace = false;
										ABattleMobaCharacter* hitChar = Cast<ABattleMobaCharacter>(hitRes5.Actor);

										if (hitChar && hitChar->InRagdoll == false && hitChar->TeamName != this->TeamName)
										{
											DoDamage(hitChar);
											bApplyHitTrace = true;
										}
									}

									else
									{
										FHitResult hitRes6;
										bool bHit6 = GetWorld()->LineTraceSingleByChannel(hitRes6, Col6->GetComponentLocation(), Col6->GetComponentLocation() + (GetActorForwardVector() * TraceDistance), ECC_PhysicsBody, AttackTraceParams);

										if (bHit6)
										{
											bApplyHitTrace = false;
											ABattleMobaCharacter* hitChar = Cast<ABattleMobaCharacter>(hitRes6.Actor);

											if (hitChar && hitChar->InRagdoll == false && hitChar->TeamName != this->TeamName)
											{
												DoDamage(hitChar);
												bApplyHitTrace = true;
											}
										}
									}
								}
							}
						}
					}
				}
			}

			//		get AttackCol world locations
		}
	}
}

bool ABattleMobaCharacter::ServerExecuteAction_Validate(FActionSkill SelectedRow, FName ActiveSection, FName MontageSection, bool bSpecialAttack)
{
	return true;
}

void ABattleMobaCharacter::ServerExecuteAction_Implementation(FActionSkill SelectedRow, FName ActiveSection, FName MontageSection, bool bSpecialAttack)
{
	MulticastExecuteAction(SelectedRow, ActiveSection, MontageSection, bSpecialAttack);
}


void ABattleMobaCharacter::OnResetVR()
{
	UHeadMountedDisplayFunctionLibrary::ResetOrientationAndPosition();
}


void ABattleMobaCharacter::TouchStarted(ETouchIndex::Type FingerIndex, FVector Location)
{
	//Jump();
}

void ABattleMobaCharacter::TouchStopped(ETouchIndex::Type FingerIndex, FVector Location)
{
	//StopJumping();
}

void ABattleMobaCharacter::OnCameraShake()
{
	CombatCamShake();
	//GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Orange, FString::Printf(TEXT("Calling Camera Shake Function")));
}

void ABattleMobaCharacter::OnRep_Team()
{
	UUserWidget* HPWidget = Cast<UUserWidget>(W_DamageOutput->GetUserWidgetObject());
	if (HPWidget)
	{
		const FName hptext = FName(TEXT("TeamName"));
		UTextBlock* HealthText = (UTextBlock*)(HPWidget->WidgetTree->FindWidget(hptext));

		if (HealthText)
		{
			HealthText->SetText(FText::FromName(TeamName));
		}
	}
}

bool ABattleMobaCharacter::TowerReceiveDamage_Validate(ADestructibleTower * Tower, float DamageApply)
{
	return true;
}

void ABattleMobaCharacter::TowerReceiveDamage_Implementation(ADestructibleTower * Tower, float DamageApply)
{
	if (Tower)
	{
		//ABattleMobaCharacter* Player = Cast<ABattleMobaCharacter>(HitActor);
		Tower->CurrentHealth = FMath::Clamp(Tower->CurrentHealth - DamageApply, 0.0f, Tower->MaxHealth);
		Tower->IsHit = false;
		Tower->OnRep_UpdateHealth();

		if (Tower->CurrentHealth <= 0.0f)
		{
			//Tower->CurrentHealth = 0.0f;
			Tower->isDestroyed = true;


			/**		Destroy Tower*/
			//Tower->SetActorHiddenInGame(true);
			Tower->OnRep_Destroy();
		}
	}
	
}

void ABattleMobaCharacter::TurnAtRate(float Rate)
{
	if (GetMesh()->SkeletalMesh != nullptr)
	{
		// calculate delta for this frame from the rate information
		AddControllerYawInput(Rate * BaseTurnRate * GetWorld()->GetDeltaSeconds());
		YawRate = Rate;
	}
}

void ABattleMobaCharacter::LookUpAtRate(float Rate)
{
	if (GetMesh()->SkeletalMesh != nullptr)
	{
		// calculate delta for this frame from the rate information
		AddControllerPitchInput(Rate * BaseLookUpRate * GetWorld()->GetDeltaSeconds());
	}
}

void ABattleMobaCharacter::MoveForward(float Value)
{
	if (GetMesh()->SkeletalMesh != nullptr)
	{
		if ((Controller != NULL) && (Value != 0.0f))
		{
			if (this->AnimInsta)
			{
				if (this->AnimInsta->CanMove && this->IsStunned == false)
				{
					// find out which way is forward
					const FRotator Rotation = Controller->GetControlRotation();
					const FRotator YawRotation(0, Rotation.Yaw, 0);

					// get forward vector
					const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
					AddMovementInput(Direction, Value);
					Rotate = false;
					FoundActors.Empty();
					currentTarget = NULL;
				}
			}
		}
	}
}

void ABattleMobaCharacter::MoveRight(float Value)
{
	if (GetMesh()->SkeletalMesh != nullptr)
	{
		if ((Controller != NULL) && (Value != 0.0f))
		{
			if (this->AnimInsta)
			{
				if (this->AnimInsta->CanMove && this->IsStunned == false)
				{
					// find out which way is right
					const FRotator Rotation = Controller->GetControlRotation();
					const FRotator YawRotation(0, Rotation.Yaw, 0);

					// get right vector 
					const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
					// add movement in that direction
					AddMovementInput(Direction, Value);
					Rotate = false;
					FoundActors.Empty();
					currentTarget = NULL;
				}
			}
		}
	}
}
//Check for overlapped enemy unit
void ABattleMobaCharacter::OnBeginOverlap(UPrimitiveComponent* OverlappedActor, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	//if (OtherActor != this)
	//{
	//	ABattleMobaCharacter* EnemyChar = Cast<ABattleMobaCharacter>(OtherActor);
	//	ADestructibleTower* EnemyTow = Cast<ADestructibleTower>(OtherActor);
	//	if ((EnemyChar != nullptr && EnemyChar->Health >=0.0f && EnemyChar->TeamName != this->TeamName))
	//	{
	//		//FoundActors.AddUnique(OtherActor);
	//		this->test = true;
	//		//GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Blue, FString::Printf(TEXT("Distance: %f"), this->GetHorizontalDistanceTo(EnemyChar)));
	//	}
	//	else if (EnemyTow != nullptr && EnemyTow->TeamName != this->TeamName)
	//	{
	//		this->test = true;
	//	}
	//}
}

void ABattleMobaCharacter::OnEndOverlap(UPrimitiveComponent* OverlappedActor, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	//if (OtherActor != this)
	//{
	//	//this->test = false;
	//	ABattleMobaCharacter* EnemyChar = Cast<ABattleMobaCharacter>(OtherActor);
	//	ADestructibleTower* EnemyTow = Cast<ADestructibleTower>(OtherActor);
	//	if ((EnemyChar != nullptr && EnemyChar->Health >= 0.0f && EnemyChar->TeamName != this->TeamName) || (EnemyTow != nullptr && EnemyTow->TeamName != this->TeamName))
	//	{
	//		if (this->GetHorizontalDistanceTo(EnemyChar) > ViewDistanceCol->GetScaledSphereRadius())
	//		{
	//			this->test = false;
	//			//FoundActors.Remove(EnemyChar);
	//			GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Blue, FString::Printf(TEXT("Removed")));
	//		}
	//		else if (this->GetHorizontalDistanceTo(EnemyTow) > ViewDistanceCol->GetScaledSphereRadius())
	//		{
	//			this->test = false;
	//			//FoundActors.Remove(EnemyChar);
	//			GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Blue, FString::Printf(TEXT("Removed")));
	//		}
	//	}
	//}
}

void ABattleMobaCharacter::RotateToTargetSetup()
{
	//Distance actor struct
	TArray<FActor_Dist> distcollection;

	//Check for eligible character actors
	for (auto& name : UGameplayStatics::GetGameState(GetWorld())->PlayerArray)
	{
		if (name->GetPawn() != nullptr && name->GetPawn()->IsActorBeingDestroyed() == false)
		{
			ABattleMobaCharacter* enemy = Cast<ABattleMobaCharacter>(name->GetPawn());
			if (enemy != this && enemy->TeamName != this->TeamName)
			{
				FoundActors.AddUnique(enemy);
			}
		}
	}
	//Check for tower actors
	for (TActorIterator<ADestructibleTower> It(GetWorld()); It; ++It)
	{
		ADestructibleTower* currentTower = *It;
		if (currentTower->TeamName != this->TeamName)
		{
			FoundActors.AddUnique(currentTower);
		}
	}

	//Look for closest target from an actor
	UInputLibrary::Distance_Sort(FoundActors, this, false, distcollection);

	ABattleMobaCharacter* EnemyChar = Cast<ABattleMobaCharacter>(distcollection[0].actor);
	ADestructibleTower* EnemyTow = Cast<ADestructibleTower>(distcollection[0].actor);
	if (distcollection[0].distance <= 200.0f)
	{
		if (EnemyChar || EnemyTow)
		{
			//set new closest actor to target
			currentTarget = distcollection[0].actor;
			FoundActors.Empty();
			Rotate = true;
			GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Blue, FString::Printf(TEXT("is close")));
		}
		else
		{
			Rotate = false;
			FoundActors.Empty();
			currentTarget = NULL;
			GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Blue, FString::Printf(TEXT("is not close")));
		}
	}
}

