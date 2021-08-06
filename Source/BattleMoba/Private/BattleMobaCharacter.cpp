// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#include "BattleMobaCharacter.h"
#include "Engine.h"
#include "Net/UnrealNetwork.h"
#include "TimerManager.h"
#include "HeadMountedDisplayFunctionLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "Camera/CameraComponent.h"
#include "Components/PrimitiveComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/InputComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Controller.h"
#include "GameFramework/SpringArmComponent.h"

//////////////////////////////////////////////////////////////////////////
// ABattleMobaCharacter
#include "InputLibrary.h"

void ABattleMobaCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ABattleMobaCharacter, TeamName);
	DOREPLIFETIME(ABattleMobaCharacter, Health);
	DOREPLIFETIME(ABattleMobaCharacter, Stamina);
	DOREPLIFETIME(ABattleMobaCharacter, AttackSection);
}

ABattleMobaCharacter::ABattleMobaCharacter()
{
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);

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

	// Note: The skeletal mesh and anim blueprint references on the Mesh component (inherited from Character) 
	// are set in the derived blueprint asset named MyCharacter (to avoid direct content references in C++)
}

//////////////////////////////////////////////////////////////////////////
// Input

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



void ABattleMobaCharacter::GetButtonSkillAction(FKey Currkeys)
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
							if (this->IsLocallyControlled())
							{
								//play the animation that visible to all clients
								ServerExecuteAction(*row);

								//setting up for cooldown properties
								FTimerHandle handle;
								FTimerDelegate TimerDelegate;

								//set the row boolean to false after finish cooldown timer
								TimerDelegate.BindLambda([row]()
								{
									UE_LOG(LogTemp, Warning, TEXT("DELAY BEFORE SETTING UP COOLDOWN TO FALSE"));
									row->isOnCD = false;
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
						ServerExecuteAction(*row);
						break;
					}
				}
				/**   current skill does not use cooldown and has multiple inputs */
				else
				{
					GEngine->AddOnScreenDebugMessage(-1, 10.0f, FColor::Emerald, FString::Printf(TEXT("Current key is %s"), ((*row->keys.ToString()))));
					if (row->SkillMoveset != nullptr)
					{
						//Get section name
						row->SectionName = row->SkillMoveset->GetSectionName(row->Section);
						
						//if (bFastAttack)
						//{
						//	if (FastCount == 2)
						//	{
						//		AttackSection = "Attack2";
						//		AttackSectionUUID = 1;
						//	}

						//	else if (FastCount == 3)
						//	{
						//		AttackSection = "Attack3";
						//		AttackSectionUUID = 2;
						//	}

						//	else
						//	{
						//		//FastCount = 1;
						//		AttackSection = "Attack1";
						//		AttackSectionUUID = 0;
						//	}
						//}

						//else
						//{
						//	if (StrongCount == 2)
						//	{
						//		AttackSection = "Attack2";
						//		AttackSectionUUID = 1;
						//	}

						//	else if (StrongCount == 3)
						//	{
						//		AttackSection = "Attack3";
						//		AttackSectionUUID = 2;
						//	}

						//	else
						//	{
						//		//StrongCount = 1;
						//		AttackSection = "Attack1";
						//		AttackSectionUUID = 0;

						//	}
						//}

					/*	if (row->keys == "LeftMouseButton")
						{
							GEngine->AddOnScreenDebugMessage(-1, 10.0f, FColor::Emerald, FString::Printf(TEXT("LeftMouseButton Clicked")));
							bFastAttack = true;
							StrongCount = 0;
							FastCount += 1;
							if (FastCount > 3)
							{
								FastCount = 1;
							}
							
						}

						else if (row->keys == "RightMouseButton")
						{
							bFastAttack = false;
							FastCount = 0;
							StrongCount += 1;
							if (StrongCount > 2)
							{
								StrongCount = 1;
							}
						}*/
						if (this->IsLocallyControlled())
						{
							//play the animation that visible to all clients
							ServerExecuteAction(*row);
						}

						//Check if the next section is exist, otherwise reset to first index of the montage sections
						if (row->Section >= (row->SkillMoveset->CompositeSections.Num() - 1))
						{
							row->Section = 0;
						}
						else
							row->Section += 1;
					}
				}
			}
		}
	}
}

bool ABattleMobaCharacter::MulticastExecuteAction_Validate(FActionSkill SelectedRow)
{
	return true;
}

void ABattleMobaCharacter::MulticastExecuteAction_Implementation(FActionSkill SelectedRow)
{
	if (SelectedRow.isOnCD)
	{
		//if current montage consumes cooldown properties
		if (SelectedRow.IsUsingCD)
		{
			GEngine->AddOnScreenDebugMessage(-1, 10.0f, FColor::Emerald, FString::Printf(TEXT("Play montage: %s"), *SelectedRow.SkillMoveset->GetName()));

			this->GetMesh()->GetAnimInstance()->Montage_Play(SelectedRow.SkillMoveset, 1.0f, EMontagePlayReturnType::MontageLength, 0.0f, true);
		}
	}
	//if current montage will affects player location
	else if (SelectedRow.UseTranslate)
	{
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

			this->LaunchCharacter(dashVector, false, false);
		});
		//start cooldown the skill
		this->GetWorldTimerManager().SetTimer(handle, TimerDelegate, montageTimer/2.0f, false);
	}
	else
	{
		//if (bFastAttack)
		//{
		//	if (FastCount == 2) 
		//	{
		//		AttackSection = "Attack2";
		//		AttackSectionUUID = 1;
		//	}

		//	else if (FastCount == 3)
		//	{
		//		AttackSection = "Attack3";
		//		AttackSectionUUID = 2;
		//	}

		//	else
		//	{
		//		//FastCount = 1;
		//		AttackSection = "Attack1";
		//		AttackSectionUUID = 0;
		//	}
		//}

		//else
		//{
		//	if (StrongCount == 2)
		//	{
		//		AttackSection = "Attack2";
		//		AttackSectionUUID = 1;
		//	}

		//	else if (StrongCount == 3)
		//	{
		//		AttackSection = "Attack3";
		//		AttackSectionUUID = 2;
		//	}

		//	else
		//	{
		//		//StrongCount = 1;
		//		AttackSection = "Attack1";
		//		AttackSectionUUID = 0;

		//	}
		//}

		/** Play Attack Montage by Section */
		GEngine->AddOnScreenDebugMessage(-1, 10.0f, FColor::Emerald, FString::Printf(TEXT("Current section is %s"), ((*SelectedRow.SectionName.ToString()))));
		PlayAnimMontage(SelectedRow.SkillMoveset, 1.0f, *SelectedRow.SectionName.ToString());

		/** Get Length of the Section being played */
		//AttackSectionLength = FastAttack->GetSectionLength(AttackSectionUUID);
	}
		
}

bool ABattleMobaCharacter::ServerExecuteAction_Validate(FActionSkill SelectedRow)
{
	return true;
}

void ABattleMobaCharacter::ServerExecuteAction_Implementation(FActionSkill SelectedRow)
{
	MulticastExecuteAction(SelectedRow);
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

void ABattleMobaCharacter::TurnAtRate(float Rate)
{
	// calculate delta for this frame from the rate information
	AddControllerYawInput(Rate * BaseTurnRate * GetWorld()->GetDeltaSeconds());
}

void ABattleMobaCharacter::LookUpAtRate(float Rate)
{
	// calculate delta for this frame from the rate information
	AddControllerPitchInput(Rate * BaseLookUpRate * GetWorld()->GetDeltaSeconds());
}

void ABattleMobaCharacter::MoveForward(float Value)
{
	if ((Controller != NULL) && (Value != 0.0f))
	{
		// find out which way is forward
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		// get forward vector
		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
		AddMovementInput(Direction, Value);
	}
}

void ABattleMobaCharacter::MoveRight(float Value)
{
	if ( (Controller != NULL) && (Value != 0.0f) )
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
