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
}

ABattleMobaCharacter::ABattleMobaCharacter()
{
	this->GetMesh()->SetVisibility(false);
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

	LeftKickCol = CreateDefaultSubobject<UCapsuleComponent>(TEXT("LeftKickCol"));
	LeftKickCol->SetupAttachment(GetMesh(), "calf_l");
	LeftKickCol->SetRelativeLocation(FVector(-15.000000f, 0.000000f, 0.000000f));
	LeftKickCol->SetRelativeRotation(FRotator(90.000000f, 0.0f, 179.999924f));
	LeftKickCol->SetCapsuleHalfHeight(28);
	LeftKickCol->SetCapsuleRadius(8);
	LeftKickCol->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);

	RightKickCol = CreateDefaultSubobject<UCapsuleComponent>(TEXT("RightKickCol"));
	RightKickCol->SetupAttachment(GetMesh(), "calf_r");
	RightKickCol->SetRelativeLocation(FVector(15.000000f, 0.000000f, 0.000000f));
	RightKickCol->SetRelativeRotation(FRotator(90.000000f, 0.0f, 179.999924f));
	RightKickCol->SetCapsuleHalfHeight(28);
	RightKickCol->SetCapsuleRadius(8);
	RightKickCol->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);

	LKickArrow = CreateDefaultSubobject<UArrowComponent>(TEXT("LKickArrow"));
	LKickArrow->SetupAttachment(LeftKickCol);
	LKickArrow->SetRelativeRotation(FRotator(0.000000f, -90.000000f, 0.000000f));

	RKickArrow = CreateDefaultSubobject<UArrowComponent>(TEXT("RKickArrow"));
	RKickArrow->SetupAttachment(RightKickCol);
	RKickArrow->SetRelativeRotation(FRotator(0.000000f, 90.000000f, 0.000000f));

	LeftPunchCol = CreateDefaultSubobject<UCapsuleComponent>(TEXT("LeftPunchCol"));
	LeftPunchCol->SetupAttachment(GetMesh(), "hand_l");
	LeftPunchCol->SetRelativeLocation(FVector(0.000000f, 0.000000f, 0.000000f));
	LeftPunchCol->SetRelativeRotation(FRotator(90.000000f, 0.000000f, 0.000000f));
	LeftPunchCol->SetCapsuleHalfHeight(22);
	LeftPunchCol->SetCapsuleRadius(5);
	LeftPunchCol->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);

	RightPunchCol = CreateDefaultSubobject<UCapsuleComponent>(TEXT("RightPunchCol"));
	RightPunchCol->SetupAttachment(GetMesh(), "hand_r");
	RightPunchCol->SetRelativeLocation(FVector(0.000000f, 0.000000f, 0.000000f));
	RightPunchCol->SetRelativeRotation(FRotator(90.000000f, 0.000000f, 0.000000f));
	RightPunchCol->SetCapsuleHalfHeight(22);
	RightPunchCol->SetCapsuleRadius(5);
	RightPunchCol->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);

	LPunchArrow = CreateDefaultSubobject<UArrowComponent>(TEXT("LPunchArrow"));
	LPunchArrow->SetupAttachment(GetMesh(), "lowerarm_l");

	RPunchArrow = CreateDefaultSubobject<UArrowComponent>(TEXT("RPunchArrow"));
	RPunchArrow->SetupAttachment(GetMesh(), "lowerarm_r");
	RPunchArrow->SetRelativeRotation(FRotator(0.000157f, -179.999084f, 0.000011f));

	//WidgetComponent
	W_DamageOutput = CreateDefaultSubobject<UWidgetComponent>(TEXT("W_DamageOutput"));
	W_DamageOutput->SetupAttachment(RootComponent);
	W_DamageOutput->SetRelativeLocation(FVector(0.000000f, 0.0f, 100.0f));
	W_DamageOutput->InitWidget();

	W_DamageOutput->SetWidgetSpace(EWidgetSpace::Screen);
	W_DamageOutput->SetDrawAtDesiredSize(true);
	//W_DamageOutput->SetVisibility(false);
	W_DamageOutput->SetGenerateOverlapEvents(false);

	TraceDistance = 2000.0f;

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
	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ACharacter::Jump);
	PlayerInputComponent->BindAction("Jump", IE_Released, this, &ACharacter::StopJumping);

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
}

void ABattleMobaCharacter::BeginPlay()
{
	Super::BeginPlay();
	//this->GetMesh()->SkeletalMesh = CharMesh;

	this->GetMesh()->SetSkeletalMesh(CharMesh, false);
	AnimInsta = Cast<UBattleMobaAnimInstance>(this->GetMesh()->GetAnimInstance());

	this->GetMesh()->SetVisibility(true);

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
				GEngine->AddOnScreenDebugMessage(-1, 10.0f, FColor::Yellow, FString::Printf(TEXT("Rotation Delta: %s"), (*RotDifference.ToString())));

				/**		Get random index from array of section names*/
				FName arr[2] = { "NormalHit01", "NormalHit02" };
				int RandInt = rand() % 2;
				//FName HitSection = arr[RandInt];
				FName HitSection = "NormalHit02";

				// right
				if (UKismetMathLibrary::InRange_FloatFloat(RotDifference.Yaw, -135.0f, -45.0f, true, true))
				{
					HitReactionClient(this, Damage, this->RightHitMoveset, HitSection);
					GEngine->AddOnScreenDebugMessage(-1, 10.0f, FColor::Yellow, FString::Printf(TEXT("Hit from RIGHT")));
				}

				// front
				else if (UKismetMathLibrary::InRange_FloatFloat(RotDifference.Yaw, -45.0f, 45.0f, true, true))
				{
					HitReactionClient(this, Damage, this->FrontHitMoveset, HitSection);
					GEngine->AddOnScreenDebugMessage(-1, 10.0f, FColor::Yellow, FString::Printf(TEXT("Hit from FRONT")));
				}

				//	left
				else if (UKismetMathLibrary::InRange_FloatFloat(RotDifference.Yaw, 45.0f, 135.0f, true, true))
				{
					HitReactionClient(this, Damage, this->LeftHitMoveset, HitSection);
					GEngine->AddOnScreenDebugMessage(-1, 10.0f, FColor::Yellow, FString::Printf(TEXT("Hit from LEFT")));
				}

				//	back
				else
				{
					HitReactionClient(this, Damage, this->BackHitMoveset, HitSection);
					GEngine->AddOnScreenDebugMessage(-1, 10.0f, FColor::Yellow, FString::Printf(TEXT("Hit from BACK")));
				}

			}
		}
	}
	return 0.0f;
}

void ABattleMobaCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (WithinVicinity)
	{
		UInputLibrary::SetUIVisibility(W_DamageOutput, this);
	}

	//if (currentTarget != nullptr && Rotate == true && test == true)
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
		/*FHitResult Hit(ForceInit);

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
					this->WithinVicinity = false;

					//ps->Death += 1;

					FTimerHandle handle;
					FTimerDelegate TimerDelegate;

					//set the row boolean to false after finish cooldown timer
					TimerDelegate.BindLambda([this]()
					{
						//Set player's death count
						ABattleMobaPlayerState* ps = Cast<ABattleMobaPlayerState>(this->GetPlayerState());
						ABattleMobaGameMode* gm = Cast<ABattleMobaGameMode>(UGameplayStatics::GetGameMode(this));
						if (gm)
						{
							gm->PlayerKilled(ps, this->DamageDealers.Last(), DamageDealers);
						}
						//for (int32 i = 0; i < this->DamageDealers.Num(); i++)
						//{
						//	if (this->DamageDealers[i] == this->DamageDealers.Last())
						//	{
						//		//Set killer Kill count
						//		ABattleMobaPlayerState* pDealer = Cast<ABattleMobaPlayerState>(this->DamageDealers.Last()->GetPlayerState());
						//		if (pDealer)
						//		{
						//			pDealer->Kill += 1;
						//		}
						//	}
						//	else
						//	{
						//		//Set assist count
						//		ABattleMobaPlayerState* pDealer = Cast<ABattleMobaPlayerState>(this->DamageDealers[i]->GetPlayerState());
						//		if (pDealer)
						//		{
						//			pDealer->Assist += 1;
						//		}
						//	}
						//}
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
				PlayAnimMontage(HitMoveset, 1.0f, MontageSection);
				//float HitDuration = this->GetMesh()->GetAnimInstance()->Montage_Play(HitMoveset, 1.0f, EMontagePlayReturnType::MontageLength, 0.0f, true);
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
		GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Orange, FString::Printf(TEXT("Particle Effect Spawned")));

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

void ABattleMobaCharacter::GetButtonSkillAction(FKey Currkeys)
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
				if (row->keys == Currkeys)
				{
					//if current skill is using cooldown
					if (row->IsUsingCD && !row->UseTranslate)
					{
						//GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Blue, FString::Printf(TEXT("row->isOnCD: %s"), row->isOnCD ? TEXT("true") : TEXT("false")));

						//if the skill is on cooldown, stop playing the animation, else play the skill animation
						if (row->isOnCD == true)
						{
							GEngine->AddOnScreenDebugMessage(-1, 10.0f, FColor::Emerald, FString::Printf(TEXT("Current %s skill is on cooldown!!"), ((*name.ToString()))));
							break;
						}
						else if (row->isOnCD == false)
						{
							row->isOnCD = true;
							GEngine->AddOnScreenDebugMessage(-1, 10.0f, FColor::Emerald, FString::Printf(TEXT("Current key is %s"), ((*row->keys.ToString()))));
							if (row->SkillMoveset != nullptr)
							{
								TargetHead = row->TargetIsHead;
								if (this->IsLocallyControlled())
								{
									DetectNearestTarget();
									AttackSection = "NormalAttack01";
									//play the animation that visible to all clients
									ServerExecuteAction(*row, CurrentSection, AttackSection, true);

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
							GEngine->AddOnScreenDebugMessage(-1, 10.0f, FColor::Emerald, FString::Printf(TEXT("Current %s skill is on cooldown!!"), ((*name.ToString()))));
							break;
						}
						else if (row->isOnCD == false)
						{
							row->isOnCD = true;
							GEngine->AddOnScreenDebugMessage(-1, 10.0f, FColor::Emerald, FString::Printf(TEXT("Current key is %s"), ((*row->keys.ToString()))));
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
									break;
								}
							}
							break;
						}
					}

					/**   current skill has combo */
					else if (row->UseSection)
					{
						GEngine->AddOnScreenDebugMessage(-1, 10.0f, FColor::Emerald, FString::Printf(TEXT("Current key is %s"), ((*row->keys.ToString()))));
						if (row->SkillMoveset != nullptr)
						{
							TargetHead = row->TargetIsHead;
							DetectNearestTarget();
							AttackCombo(*row);
							break;

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
					GEngine->AddOnScreenDebugMessage(-1, 10.0f, FColor::Emerald, FString::Printf(TEXT(" bCombo Attack resets to false")));
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
		ABattleMobaPlayerState* PS = Cast<ABattleMobaPlayerState>(PC->PlayerState);
		if (PS)
		{
			PC->RespawnPawn(PS->SpawnTransform);
			PC->UnPossess();
		}
	}
}

void ABattleMobaCharacter::EnableMovementMode()
{
	if (GetMesh()->SkeletalMesh != nullptr)
	{
		bEnableMove = true;
		Rotate = false;

		if (this->AnimInsta)
		{
			this->AnimInsta->CanMove = true;
		}
	}
}

bool ABattleMobaCharacter::DetectNearestTarget_Validate()
{
	return true;
}

void ABattleMobaCharacter::DetectNearestTarget_Implementation()
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

	//		draw a purple collision sphere for 2 seconds
	DrawDebugSphere(GetWorld(), GetActorLocation(), SphereCol.GetSphereRadius(), 50, FColor::Purple, true, 2.0f);

	//		check if something got hit in the sweep
	bool isHit = GetWorld()->SweepMultiByChannel(hitResults, Start, End, FQuat::Identity, ECC_PhysicsBody, SphereCol, TraceParams);

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
							GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, FString::Printf(TEXT("Closest Actor: %s"), *closestActor->GetName()));

						}
					}

					else
					{
						closestActor = Cast<AActor>(Hit.Actor);
						GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, FString::Printf(TEXT("Closest Actor: %s"), *closestActor->GetName()));
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
			GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, FString::Printf(TEXT("Hit Result: %s"), *Hit.Actor->GetName()));
		}
		
		RotateNearestTarget(closestActor);
	}
}

bool ABattleMobaCharacter::RotateNearestTarget_Validate(AActor* Target)
{
	return true;
}

void ABattleMobaCharacter::RotateNearestTarget_Implementation(AActor* Target)
{
	if (IsValid(Target))
	{
		FLatentActionInfo LatentInfo = FLatentActionInfo();
		LatentInfo.CallbackTarget = this;

		FRotator LookRotation = UKismetMathLibrary::FindLookAtRotation(this->GetCapsuleComponent()->GetComponentLocation(), Target->GetActorLocation());
		FRotator RotateTo = FRotator(this->GetCapsuleComponent()->GetComponentRotation().Pitch, LookRotation.Yaw, this->GetCapsuleComponent()->GetComponentRotation().Roll);
		//FMath::RInterpTo(this->GetCapsuleComponent()->GetComponentRotation(), RotateTo, this->GetWorld()->GetDeltaSeconds(), 100.0f);

		//this->SetActorRotation(RotateTo);
		UKismetSystemLibrary::MoveComponentTo(this->GetCapsuleComponent(), this->GetCapsuleComponent()->GetComponentLocation(), RotateTo, true, true, 0.0f, true, EMoveComponentAction::Type::Move, LatentInfo);
		closestActor = nullptr;


	}
}


void ABattleMobaCharacter::SafeZone(ABMobaTriggerCapsule* TriggerZone)
{
	if (IsLocallyControlled())
	{
		//Run server safezone
		SafeZoneServer(TriggerZone);
	}
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
	GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Orange, FString::Printf(TEXT("SAFE ZONE??????????")));
	
	TriggerZone->val = TriggerZone->val + 1;
	TriggerZone->OnRep_Val();
}

void ABattleMobaCharacter::ControlFlagMode(ABattleMobaCTF * cf)
{
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

void ABattleMobaCharacter::ControlFlagServer_Implementation(ABattleMobaCTF * cf)
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

bool ABattleMobaCharacter::ControlFlagMulticast_Validate(ABattleMobaCTF * cf, FName Team)
{
	return true;
}

void ABattleMobaCharacter::ControlFlagMulticast_Implementation(ABattleMobaCTF * cf, FName Team)
{
	if (Team == "Radiant")
	{
		//		Decrease the valDire if exists first before increasing valRadiant
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
		GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Blue, FString::Printf(TEXT("Player %s with %s Widget"), *GetDebugName(this), *HPWidget->GetFName().ToString()));
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
			GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Blue, FString::Printf(TEXT("row->isOnCD: %s"), SelectedRow.isOnCD ? TEXT("true") : TEXT("false")));

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


					GEngine->AddOnScreenDebugMessage(-1, 10.0f, FColor::Emerald, FString::Printf(TEXT("Play montage: %s"), *SelectedRow.SkillMoveset->GetName()));
					GEngine->AddOnScreenDebugMessage(-1, 10.0f, FColor::Emerald, FString::Printf(TEXT("ISUSINGCD")));

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
						GEngine->AddOnScreenDebugMessage(-1, 10.0f, FColor::Emerald, FString::Printf(TEXT("Play montage: %s"), *SelectedRow.SkillMoveset->GetName()));

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
					/*if (this->IsLocallyControlled())
					{
						if (test == true)
						{
							RotateToTargetSetup();
						}
					}*/
					
					
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
					GEngine->AddOnScreenDebugMessage(-1, 10.0f, FColor::Emerald, FString::Printf(TEXT("STATEMENT ELSE")));
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
		GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Magenta, FString::Printf(TEXT("Damage Applied: %f"), this->ActualDamage));
		UE_LOG(LogTemp, Warning, TEXT("Damage Applied: %f"), this->ActualDamage);
		this->ActualDamage = UGameplayStatics::ApplyDamage(HitActor, this->ActualDamage, nullptr, this, nullptr);
	}
}

bool ABattleMobaCharacter::FireTrace_Validate(FVector StartPoint, FVector EndPoint, bool Head)
{
	return true;
}

void ABattleMobaCharacter::FireTrace_Implementation(FVector StartPoint, FVector EndPoint, bool Head)
{
	GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Red, FString::Printf(TEXT("Target Is Head: %s"), Head? TEXT("true") : TEXT("false")));
	//GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Magenta, FString::Printf(TEXT("Enter Fire Trace")));
	//Hit result storage
	FHitResult HitRes;

	FVector loc;
	FRotator rot;
	FHitResult hit;

	//GetController()->GetPlayerViewPoint(loc, rot);

	//FVector dashVector = FVector(this->GetCapsuleComponent()->GetForwardVector().X*SelectedRow.TranslateDist, this->GetCapsuleComponent()->GetForwardVector().Y*SelectedRow.TranslateDist, this->GetCapsuleComponent()->GetForwardVector().Z);

	FVector Start;
	FVector End;
	
	if (Head == true)
	{
		Start = FVector(this->BaseArrow->GetComponentLocation().X, this->BaseArrow->GetComponentLocation().Y, this->BaseArrow->GetComponentLocation().Z + 50.0f);
		End = this->GetCapsuleComponent()->GetForwardVector()*TraceDistance + Start;
	}
	else
	{
		Start = FVector(this->BaseArrow->GetComponentLocation().X, this->BaseArrow->GetComponentLocation().Y, this->BaseArrow->GetComponentLocation().Z);
		End = this->GetCapsuleComponent()->GetForwardVector()*TraceDistance + Start;
	}

	/*FVector Start = FVector(this->GetCapsuleComponent()->GetForwardVector());
	FVector End = Start + (rot.Vector()*TraceDistance);*/

	FCollisionQueryParams TraceParams;
	TraceParams.AddIgnoredActor(this);

	bool bHit = GetWorld()->LineTraceSingleByChannel(hit, Start, End, ECC_PhysicsBody, TraceParams);

	DrawDebugLine(GetWorld(), Start, End, FColor::Red, false, 2.0f);

	if (bHit)
	{
		//GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Orange, FString::Printf(TEXT("Is not self")));
		ABattleMobaCharacter* hitChar = Cast<ABattleMobaCharacter>(hit.Actor);

		TracedChar = hitChar;
		TowerActor = Cast<ADestructibleTower>(hit.Actor);

		if (hitChar && hitChar->InRagdoll == false && hitChar->TeamName != this->TeamName)
		{
			if (DoOnce == false)
			{
				/**		Debug line trace elements*/
				DoOnce = true;
				DrawDebugBox(GetWorld(), hit.ImpactPoint, FVector(5, 5, 5), FColor::Emerald, false, 2.0f);
				hitChar->HitLocation = hit.Location;
				hitChar->BoneName = hit.BoneName;
				hitChar->IsHit = true;
				hitChar->AttackerLocation = this->GetActorLocation();
				hitChar->HitEffect = this->HitEffect;

				/**		attacking a player who is on Special Attack 2 (Counter Attack)*/
				if (hitChar->AnimInsta->Montage_IsPlaying(hitChar->CounterMoveset))
				{
					ServerRotateHitActor(hitChar, this);
					ServerCounterAttack(hitChar);
					
				}

				else
				{
					GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Magenta, FString::Printf(TEXT("Bone: %s"), *hitChar->BoneName.ToString()));
					GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Magenta, FString::Printf(TEXT("Bone: %s"), *hit.GetComponent()->GetName()));
					GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Yellow, FString::Printf(TEXT("Impact: %s"), *hitChar->HitLocation.ToString()));
					GEngine->AddOnScreenDebugMessage(-1, 10.0f, FColor::Red, FString::Printf(TEXT("Blocking hit is %s"), (hitChar->IsHit) ? TEXT("True") : TEXT("False")));
					GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Green, FString::Printf(TEXT("You are hitting: %s"), *UKismetSystemLibrary::GetDisplayName(hitChar)));
					GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Orange, FString::Printf(TEXT("hitchar exist")));

					/**		set the hitActor hit movesets from the same row of skill moveset the attacker used*/
					hitChar->HitReactionMoveset = this->HitReactionMoveset;
					hitChar->FrontHitMoveset = this->FrontHitMoveset;
					hitChar->BackHitMoveset = this->BackHitMoveset;
					hitChar->LeftHitMoveset = this->LeftHitMoveset;
					hitChar->RightHitMoveset = this->RightHitMoveset;

					//Apply damage
					DoDamage(hitChar);
				}
				
			}
		}

		/**		if Actor hit by line trace is Destructible Tower*/
		else if (TowerActor && TowerActor->isDestroyed == false && TowerActor->TeamName != this->TeamName)
		{
			if (DoOnce == false)
			{
				DoOnce = true;
				GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Orange, FString::Printf(TEXT("HIT TOWER!!!!!!!!!!!!!!!!!!!!!!")));
				DrawDebugBox(GetWorld(), hit.ImpactPoint, FVector(5, 5, 5), FColor::Emerald, false, 2.0f);
				TowerActor->IsHit = true;
				TowerReceiveDamage(TowerActor, this->BaseDamage);
			}
		}
		else
			GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Green, FString::Printf(TEXT("Invalid target: %s"), *UKismetSystemLibrary::GetDisplayName(hitChar)));
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
	Jump();
}

void ABattleMobaCharacter::TouchStopped(ETouchIndex::Type FingerIndex, FVector Location)
{
	StopJumping();
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
	////Distance actor struct
	//TArray<FActor_Dist> distcollection;

	////Check for eligible character actors
	//for (auto& name : UGameplayStatics::GetGameState(GetWorld())->PlayerArray)
	//{
	//	if (name->GetPawn() != nullptr && name->GetPawn()->IsActorBeingDestroyed() == false)
	//	{
	//		ABattleMobaCharacter* enemy = Cast<ABattleMobaCharacter>(name->GetPawn());
	//		if (enemy != this && enemy->TeamName != this->TeamName)
	//		{
	//			FoundActors.AddUnique(enemy);
	//		}
	//	}
	//}
	////Check for tower actors
	//for (TActorIterator<ADestructibleTower> It(GetWorld()); It; ++It)
	//{
	//	ADestructibleTower* currentTower = *It;
	//	if (currentTower->TeamName != this->TeamName)
	//	{
	//		FoundActors.AddUnique(currentTower);
	//	}
	//}

	////Look for closest target from an actor
	//UInputLibrary::Distance_Sort(FoundActors, this, false, distcollection);

	//ABattleMobaCharacter* EnemyChar = Cast<ABattleMobaCharacter>(distcollection[0].actor);
	//ADestructibleTower* EnemyTow = Cast<ADestructibleTower>(distcollection[0].actor);
	//if (EnemyChar || EnemyTow)
	//{
	//	//set new closest actor to target
	//	currentTarget = distcollection[0].actor;
	//	Rotate = true;
	//	GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Blue, FString::Printf(TEXT("is close")));
	//}
	//else
	//{
	//	Rotate = false;
	//	currentTarget = NULL;
	//	GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Blue, FString::Printf(TEXT("is not close")));
	//}
}

