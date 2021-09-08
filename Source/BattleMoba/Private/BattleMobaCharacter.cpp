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

//////////////////////////////////////////////////////////////////////////
// ABattleMobaCharacter
#include "InputLibrary.h"
#include "BattleMobaAnimInstance.h"
#include "BattleMobaPC.h"
#include "DestructibleTower.h"
#include "BattleMobaGameState.h"
#include "BattleMobaPlayerState.h"
#include "BattleMobaGameMode.h"


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
	DOREPLIFETIME(ABattleMobaCharacter, TargetHead);
	DOREPLIFETIME(ABattleMobaCharacter, DamageDealers);
	DOREPLIFETIME(ABattleMobaCharacter, Rotate);
	DOREPLIFETIME(ABattleMobaCharacter, AttackerLocation);
	DOREPLIFETIME(ABattleMobaCharacter, CharMesh);
	DOREPLIFETIME(ABattleMobaCharacter, currentTarget);
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
			HealthBar->SetPercent(FMath::Clamp(this->Health / 100.0f, 0.0f, 1.0f));
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

	this->GetMesh()->SetSkeletalMesh(CharMesh, false);
	this->GetMesh()->SetVisibility(true);
	AnimInsta = Cast<UBattleMobaAnimInstance>(this->GetMesh()->GetAnimInstance());

	/*if (this->GetMesh()->IsVisible())
	{
		AnimInsta = Cast<UBattleMobaAnimInstance>(this->GetMesh()->GetAnimInstance());
	}*/

	FString Context;
	for (auto& name : ActionTable->GetRowNames())
	{
		FActionSkill* row = ActionTable->FindRow<FActionSkill>(name, Context);

		if (row)
		{
			row->isOnCD = false;
		}
	}
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
			HitReactionClient(this, Damage, EnemyHitReactionMoveset);
		}
	}
	return 0.0f;
}

void ABattleMobaCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	FHitResult Hit(ForceInit);

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
	}

	if (currentTarget != nullptr && Rotate == true && test == true)
	{
		if (HasAuthority())
		{
			if (this->IsLocallyControlled())
			{
				FRotator RotatorVal = UKismetMathLibrary::FindLookAtRotation(this->GetCapsuleComponent()->GetComponentLocation(), currentTarget->GetActorLocation());
				FRotator FinalVal = FRotator(this->GetCapsuleComponent()->GetComponentRotation().Pitch, RotatorVal.Yaw, this->GetCapsuleComponent()->GetComponentRotation().Roll);
				FMath::RInterpTo(this->GetCapsuleComponent()->GetComponentRotation(), FinalVal, GetWorld()->GetDeltaSeconds(), 30.0f);
				this->SetActorRotation(FinalVal);
				RotateToCameraView(FinalVal);
				//this->SetActorRotation(FRotator(this->GetActorRotation().Pitch, this->GetControlRotation().Yaw, this->GetActorRotation().Roll));
				//
				//Set rotate to false
				/*FTimerHandle handle;
				FTimerDelegate TimerDelegate;
				TimerDelegate.BindLambda([this]()
				{
					Rotate = false;
					GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Blue, FString::Printf(TEXT("Rotate: %s"), Rotate ? TEXT("true") : TEXT("false")));
				});
				this->GetWorldTimerManager().SetTimer(handle, TimerDelegate, 1.0f, false);*/
			}
		}
		else
		{
			if (this->GetController() != nullptr)
			{
				/*ServerRotateToCameraView(FRotator(this->GetActorRotation().Pitch, this->GetControlRotation().Yaw, this->GetActorRotation().Roll));
				this->SetActorRotation(FRotator(this->GetActorRotation().Pitch, this->GetControlRotation().Yaw, this->GetActorRotation().Roll));*/
				FRotator RotatorVal = UKismetMathLibrary::FindLookAtRotation(this->GetCapsuleComponent()->GetComponentLocation(), currentTarget->GetActorLocation());
				FRotator FinalVal = FRotator(this->GetCapsuleComponent()->GetComponentRotation().Pitch, RotatorVal.Yaw, this->GetCapsuleComponent()->GetComponentRotation().Roll);
				FMath::RInterpTo(this->GetCapsuleComponent()->GetComponentRotation(), FinalVal, GetWorld()->GetDeltaSeconds(), 30.0f);
				ServerRotateToCameraView(FinalVal);
				this->SetActorRotation(FinalVal);

				//Set rotate to false
				/*FTimerHandle handle;
				FTimerDelegate TimerDelegate;
				TimerDelegate.BindLambda([this]()
				{
					Rotate = false;
					GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Blue, FString::Printf(TEXT("Rotate: %s"), Rotate ? TEXT("true") : TEXT("false")));
				});
				this->GetWorldTimerManager().SetTimer(handle, TimerDelegate, 1.0f, false);
				GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Blue, FString::Printf(TEXT("Rotate: %s"), Rotate ? TEXT("true") : TEXT("false")));*/
			}
		}
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
	/*FTimerHandle handle;
	FTimerDelegate TimerDelegate;*/

	//set the row boolean to false after finish cooldown timer
	/*TimerDelegate.BindLambda([this]()
	{
		Rotate = false;
		GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Blue, FString::Printf(TEXT("Rotate: %s"), Rotate ? TEXT("true") : TEXT("false")));
	});
	this->GetWorldTimerManager().SetTimer(handle, TimerDelegate, 1.0f, false);*/
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

bool ABattleMobaCharacter::HitReactionServer_Validate(AActor * HitActor, float DamageReceived, UAnimMontage* HitMoveset)
{
	return true;
}

void ABattleMobaCharacter::HitReactionServer_Implementation(AActor* HitActor, float DamageReceived, UAnimMontage* HitMoveset)
{
	if (this->GetLocalRole() == ROLE_Authority)
	{
		if (HitActor == this)
		{
			HitReactionClient(HitActor, DamageReceived, HitMoveset);
		}
	}
}

bool ABattleMobaCharacter::HitReactionClient_Validate(AActor* HitActor, float DamageReceived, UAnimMontage* HitMoveset)
{
	return true;
}

void ABattleMobaCharacter::HitReactionClient_Implementation(AActor* HitActor, float DamageReceived, UAnimMontage* HitMoveset)
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

				/**		Force player to face Attacker*/
				FRotator PlayerRot = UKismetMathLibrary::FindLookAtRotation(this->GetActorLocation(), AttackerLocation);
				FRotator NewRot = FMath::RInterpTo(this->GetActorRotation(), PlayerRot, GetWorld()->GetDeltaSeconds(), 200.0f);
				FRotator NewRot2 = FRotator(this->GetActorRotation().Pitch, NewRot.Yaw, this->GetActorRotation().Roll);
				this->SetActorRotation(NewRot2);

				/**		Play hit reaction animation on hit*/
				float HitDuration = this->GetMesh()->GetAnimInstance()->Montage_Play(HitMoveset, 1.0f, EMontagePlayReturnType::MontageLength, 0.0f, true);
				this->IsHit = false;
				OnRep_Health();

				//run clear damage dealers array
				if (this->GetWorldTimerManager().IsTimerActive(this->DealerTimer))
				{
					this->GetWorldTimerManager().ClearTimer(this->DealerTimer);
				}
				this->GetWorldTimerManager().SetTimer(this->DealerTimer, this, &ABattleMobaCharacter::ClearDamageDealers, 5.0f, true);
			}
		}
	}
	UpdateHUD();
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
					if (row->IsUsingCD)
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
									//play the animation that visible to all clients
									ServerExecuteAction(*row, AttackSection);

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
					else if (row->UseTranslate)
					{
						GEngine->AddOnScreenDebugMessage(-1, 10.0f, FColor::Emerald, FString::Printf(TEXT("Current key is %s"), ((*row->keys.ToString()))));
						if (this->IsLocallyControlled())
						{
							//play the animation that visible to all clients
							ServerExecuteAction(*row, AttackSection);
							break;
						}
					}
					/**   current skill does not use cooldown and has multiple inputs */
					else if (row->UseSection)
					{
						GEngine->AddOnScreenDebugMessage(-1, 10.0f, FColor::Emerald, FString::Printf(TEXT("Current key is %s"), ((*row->keys.ToString()))));
						if (row->SkillMoveset != nullptr)
						{
							TargetHead = row->TargetIsHead;

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
		/**		Continues combo section of the montage if there is montage playing*/
		//if (AnimInsta->Montage_IsPlaying(FastComboMoveset)||(AnimInsta->Montage_IsPlaying(StrongkComboMoveset)))
		if (AnimInsta->IsAnyMontagePlaying())
		{
			if (SelectedRow.Section == 3)
			{
				/**		Checks whether the current montage section contains "Combo" substring*/
				FName CurrentSection = AnimInsta->Montage_GetCurrentSection(AnimInsta->GetCurrentActiveMontage());
				if (UKismetStringLibrary::Contains(CurrentSection.ToString(), TEXT("Combo"), false, false))
				{
					/**		Checks if current combo section contains "01" substring*/
					if (UKismetStringLibrary::Contains(CurrentSection.ToString(), TEXT("01"), false, false))
					{
						FString NextSection = UKismetStringLibrary::Concat_StrStr(TEXT("NormalAttack"), TEXT("02"));
						AttackSection = FName(*NextSection);
						if (IsLocallyControlled())
						{
							/**		Change next attack to combo montage section*/
							ServerExecuteAction(SelectedRow, AttackSection);
						}
					}

					else if (UKismetStringLibrary::Contains(CurrentSection.ToString(), TEXT("02"), false, false))
					{
						FString NextSection = UKismetStringLibrary::Concat_StrStr(TEXT("NormalAttack"), TEXT("03"));
						AttackSection = FName(*NextSection);
						if (IsLocallyControlled())
						{
							/**		Change next attack to combo montage section*/
							ServerExecuteAction(SelectedRow, AttackSection);
						}
					}

					else if (UKismetStringLibrary::Contains(CurrentSection.ToString(), TEXT("03"), false, false))
					{
						FString NextSection = UKismetStringLibrary::Concat_StrStr(TEXT("NormalAttack"), TEXT("01"));
						AttackSection = FName(*NextSection);
						if (IsLocallyControlled())
						{
							/**		Change next attack to combo montage section*/
							ServerExecuteAction(SelectedRow, AttackSection);
						}
					}
				}
			}

			else if (SelectedRow.Section == 2)
			{
				/**		Checks whether the current montage section contains "Combo" substring*/
				FName CurrentSection = AnimInsta->Montage_GetCurrentSection(AnimInsta->GetCurrentActiveMontage());
				if (UKismetStringLibrary::Contains(CurrentSection.ToString(), TEXT("Combo"), false, false))
				{
					/**		Checks if current combo section contains "01" substring*/
					if (UKismetStringLibrary::Contains(CurrentSection.ToString(), TEXT("01"), false, false))
					{
						FString NextSection = UKismetStringLibrary::Concat_StrStr(TEXT("NormalAttack"), TEXT("02"));
						AttackSection = FName(*NextSection);

						if (IsLocallyControlled())
						{
							/**		Change next attack to combo montage section*/
							ServerExecuteAction(SelectedRow, AttackSection);
						}
					}

					else if (UKismetStringLibrary::Contains(CurrentSection.ToString(), TEXT("02"), false, false))
					{
						FString NextSection = UKismetStringLibrary::Concat_StrStr(TEXT("NormalAttack"), TEXT("01"));
						AttackSection = FName(*NextSection);

						if (IsLocallyControlled())
						{
							/**		Change next attack to combo montage section*/
							ServerExecuteAction(SelectedRow, AttackSection);
						}
					}

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
			if (IsLocallyControlled())
			{
				ServerExecuteAction(SelectedRow, AttackSection);
			}

			float SectionLength = SelectedRow.SkillMoveset->GetSectionLength(0);

			TimerDelegate.BindLambda([this]()
			{
				bAttacking = false;
				GEngine->AddOnScreenDebugMessage(-1, 10.0f, FColor::Emerald, FString::Printf(TEXT(" bCombo Attack resets to false")));
			});

			/**		Reset boolean after section ends*/
			this->GetWorldTimerManager().SetTimer(Timer, TimerDelegate, SectionLength, false);
		}
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

		if (this->AnimInsta)
		{
			this->AnimInsta->CanMove = true;
		}
	}
}

void ABattleMobaCharacter::SafeZone(const FString& NewText)
{
	if (IsLocallyControlled())
	{
		SafeZoneServer(NewText);
	}
}

bool ABattleMobaCharacter::SafeZoneServer_Validate(const FString& NewText)
{
	return true;
}

void ABattleMobaCharacter::SafeZoneServer_Implementation(const FString& NewText)
{
	SafeZoneMulticast(NewText);
}

bool ABattleMobaCharacter::SafeZoneMulticast_Validate(const FString& NewText)
{
	return true;
}

void ABattleMobaCharacter::SafeZoneMulticast_Implementation(const FString& NewText)
{
	GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Red, FString::Printf(TEXT("%s"), *NewText));
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

bool ABattleMobaCharacter::MulticastExecuteAction_Validate(FActionSkill SelectedRow, FName MontageSection)
{
	return true;
}

void ABattleMobaCharacter::MulticastExecuteAction_Implementation(FActionSkill SelectedRow, FName MontageSection)
{
	if (GetMesh()->SkeletalMesh != nullptr)
	{
		GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Blue, FString::Printf(TEXT("row->isOnCD: %s"), SelectedRow.isOnCD ? TEXT("true") : TEXT("false")));

		/**		Disable movement on Action Skill*/
		if (AnimInsta != nullptr)
		{
			AnimInsta->CanMove = false;
		}

		FTimerHandle Delay;

		if (SelectedRow.isOnCD)
		{
			if (test == true)
			{
				RotateToTargetSetup();
			}

			//if current montage consumes cooldown properties
			if (SelectedRow.IsUsingCD)
			{
				GEngine->AddOnScreenDebugMessage(-1, 10.0f, FColor::Emerald, FString::Printf(TEXT("Play montage: %s"), *SelectedRow.SkillMoveset->GetName()));
				GEngine->AddOnScreenDebugMessage(-1, 10.0f, FColor::Emerald, FString::Printf(TEXT("ISUSINGCD")));
				float montageDuration = this->GetMesh()->GetAnimInstance()->Montage_Play(SelectedRow.SkillMoveset, 1.0f, EMontagePlayReturnType::MontageLength, 0.0f, true);

				/**		Enable movement back after montage finished playing*/
				this->GetWorld()->GetTimerManager().SetTimer(Delay, this, &ABattleMobaCharacter::EnableMovementMode, montageDuration, false);

			}
		}
		//if current montage will affects player location
		else if (SelectedRow.UseTranslate)
		{
			/*if (Rotate == true)
			{
				Rotate = false;
			}*/
			GEngine->AddOnScreenDebugMessage(-1, 10.0f, FColor::Emerald, FString::Printf(TEXT("Play montage: %s"), *SelectedRow.SkillMoveset->GetName()));

			float montageTimer = this->GetMesh()->GetAnimInstance()->Montage_Play(SelectedRow.SkillMoveset, 1.0f, EMontagePlayReturnType::MontageLength, 0.0f, true);

			//setting up for translate properties
			FTimerHandle handle;
			FTimerDelegate TimerDelegate;

			//launch player forward after finish montage timer
			TimerDelegate.BindLambda([this, SelectedRow]()
			{
				UE_LOG(LogTemp, Warning, TEXT("DELAY BEFORE TRANSLATE CHARACTER FORWARD"));

				FVector dashVector = FVector(this->GetCapsuleComponent()->GetForwardVector().X*SelectedRow.TranslateDist, this->GetCapsuleComponent()->GetForwardVector().Y*SelectedRow.TranslateDist, this->GetCapsuleComponent()->GetForwardVector().Z);

				this->LaunchCharacter(dashVector, true, true);
			});
			//start cooldown the skill
			this->GetWorldTimerManager().SetTimer(handle, TimerDelegate, 0.218f, false);

			/**		Enable movement back after montage finished playing*/
			this->GetWorld()->GetTimerManager().SetTimer(Delay, this, &ABattleMobaCharacter::EnableMovementMode, montageTimer, false);
		}

		else if (SelectedRow.UseSection)
		{
			if (IsLocallyControlled() == true)
			{
				if (test == true)
				{
					RotateToTargetSetup();
				}
			}
			/** Play Attack Montage by Section */
			GEngine->AddOnScreenDebugMessage(-1, 10.0f, FColor::Emerald, FString::Printf(TEXT("Play montage: %s"), *SelectedRow.SkillMoveset->GetName()));
			GEngine->AddOnScreenDebugMessage(-1, 10.0f, FColor::Emerald, FString::Printf(TEXT("Current Section is %s"), *MontageSection.ToString()));
			PlayAnimMontage(SelectedRow.SkillMoveset, 1.0f, MontageSection);
			SelectedRow.SkillMoveset->GetSectionLength(AttackSectionUUID);
		}

		else
		{
			//Rotate = false;
			GEngine->AddOnScreenDebugMessage(-1, 10.0f, FColor::Emerald, FString::Printf(TEXT("STATEMENT ELSE")));
		}

		this->damage = SelectedRow.Damage;
		this->HitReactionMoveset = SelectedRow.HitMoveset;
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
		//ApplyDamage
		this->damage = UGameplayStatics::ApplyDamage(HitActor, this->damage, nullptr, this, nullptr);
		GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Magenta, FString::Printf(TEXT("Damage Applied: %f"), this->damage));
		UE_LOG(LogTemp, Warning, TEXT("Damage Applied: %f"), this->damage);
		//DrawDebugSphere(GetWorld(), Start, SphereKick.GetSphereRadius(), 2, FColor::Purple, false, 1, 0, 1);
		//reset the bool so sweep trace can be executed again
		//DoOnce = false;
		//LeftKickColActivate = false;
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
				//Apply damage
				DoOnce = true;
				DrawDebugBox(GetWorld(), hit.ImpactPoint, FVector(5, 5, 5), FColor::Emerald, false, 2.0f);
				hitChar->HitLocation = hit.Location;
				hitChar->BoneName = hit.BoneName;
				hitChar->IsHit = true;
				hitChar->AttackerLocation = this->GetActorLocation();

				GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Magenta, FString::Printf(TEXT("Bone: %s"), *hitChar->BoneName.ToString()));
				GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Magenta, FString::Printf(TEXT("Bone: %s"), *hit.GetComponent()->GetName()));
				GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Yellow, FString::Printf(TEXT("Impact: %s"), *hitChar->HitLocation.ToString()));
				GEngine->AddOnScreenDebugMessage(-1, 10.0f, FColor::Red, FString::Printf(TEXT("Blocking hit is %s"), (hitChar->IsHit) ? TEXT("True") : TEXT("False")));
				GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Green, FString::Printf(TEXT("You are hitting: %s"), *UKismetSystemLibrary::GetDisplayName(hitChar)));
				GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Orange, FString::Printf(TEXT("hitchar exist")));
				hitChar->EnemyHitReactionMoveset = this->HitReactionMoveset;
				DoDamage(hitChar);
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
				TowerReceiveDamage(TowerActor, this->damage);
			}
		}
		else
			GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Green, FString::Printf(TEXT("Invalid target: %s"), *UKismetSystemLibrary::GetDisplayName(hitChar)));
	}
}

bool ABattleMobaCharacter::ServerExecuteAction_Validate(FActionSkill SelectedRow, FName MontageSection)
{
	return true;
}

void ABattleMobaCharacter::ServerExecuteAction_Implementation(FActionSkill SelectedRow, FName MontageSection)
{
	MulticastExecuteAction(SelectedRow, MontageSection);
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
				if (this->AnimInsta->CanMove)
				{
					// find out which way is forward
					const FRotator Rotation = Controller->GetControlRotation();
					const FRotator YawRotation(0, Rotation.Yaw, 0);

					// get forward vector
					const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
					AddMovementInput(Direction, Value);
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
				if (this->AnimInsta->CanMove)
				{
					// find out which way is right
					const FRotator Rotation = Controller->GetControlRotation();
					const FRotator YawRotation(0, Rotation.Yaw, 0);

					// get right vector 
					const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
					// add movement in that direction
					AddMovementInput(Direction, Value);
				}
			}
		}
	}
}

void ABattleMobaCharacter::OnBeginOverlap(UPrimitiveComponent* OverlappedActor, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (OtherActor != this)
	{
		ABattleMobaCharacter* EnemyChar = Cast<ABattleMobaCharacter>(OtherActor);
		ADestructibleTower* EnemyTow = Cast<ADestructibleTower>(OtherActor);
		if ((EnemyChar != nullptr && EnemyChar->TeamName != this->TeamName) || (EnemyTow != nullptr && EnemyTow->TeamName != this->TeamName))
		{
			this->test = true;
			//GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Blue, FString::Printf(TEXT("Distance: %f"), this->GetHorizontalDistanceTo(EnemyChar)));
		}
	}
}

void ABattleMobaCharacter::OnEndOverlap(UPrimitiveComponent* OverlappedActor, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (OtherActor != this)
	{
		ABattleMobaCharacter* EnemyChar = Cast<ABattleMobaCharacter>(OtherActor);
		ADestructibleTower* EnemyTow = Cast<ADestructibleTower>(OtherActor);
		if ((EnemyChar != nullptr && EnemyChar->TeamName != this->TeamName) || (EnemyTow != nullptr && EnemyTow->TeamName != this->TeamName))
		{
			this->test = false;
		}
	}
}

void ABattleMobaCharacter::RotateToTargetSetup()
{
	//Check for eligible actors
	for (TActorIterator<AActor> It(GetWorld()); It; ++It)
	{
		if (*It != this)
		{
			ABattleMobaCharacter* EnemyChar = Cast<ABattleMobaCharacter>(*It);
			ADestructibleTower* EnemyTow = Cast<ADestructibleTower>(*It);
			if (EnemyChar || EnemyTow)
			{
				//if distance is below appropriate value, set rotate to true
				if (this->GetHorizontalDistanceTo(*It) < 120.0f)
				{
					currentTarget = *It;
					Rotate = true;
					break;
				}
				else
					Rotate = false;
			}
		}
	}
}
