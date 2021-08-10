// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/PrimitiveComponent.h"
#include "InputLibrary.h"
#include "GameFramework/Character.h"
#include "BattleMobaCharacter.generated.h"

UCLASS(config=Game)
class ABattleMobaCharacter : public ACharacter
{
	GENERATED_BODY()

	//Replicated Network setup
	void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	/** Camera boom positioning the camera behind the character */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class USpringArmComponent* CameraBoom;

	/** Follow camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class UCameraComponent* FollowCamera;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = Components, meta = (AllowPrivateAccess = "true"))
		class UCapsuleComponent* LeftKickCol;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = Components, meta = (AllowPrivateAccess = "true"))
		class UCapsuleComponent* RightKickCol;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Components, meta = (AllowPrivateAccess = "true"))
		class UArrowComponent* LKickArrow;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Components, meta = (AllowPrivateAccess = "true"))
		class UArrowComponent* RKickArrow;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = Components, meta = (AllowPrivateAccess = "true"))
		class UCapsuleComponent* LeftPunchCol;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = Components, meta = (AllowPrivateAccess = "true"))
		class UCapsuleComponent* RightPunchCol;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Components, meta = (AllowPrivateAccess = "true"))
		class UArrowComponent* LPunchArrow;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Components, meta = (AllowPrivateAccess = "true"))
		class UArrowComponent* RPunchArrow;

public:
	ABattleMobaCharacter();

	/** Base turn rate, in deg/sec. Other scaling may affect final turn rate. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category=Camera)
	float BaseTurnRate;

	/** Base look up/down rate, in deg/sec. Other scaling may affect final rate. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category=Camera)
	float BaseLookUpRate;

	



protected:

	/** Called for forwards/backward input */
	void MoveForward(float Value);

	/** Called for side to side input */
	void MoveRight(float Value);

	/** 
	 * Called via input to turn at a given rate. 
	 * @param Rate	This is a normalized rate, i.e. 1.0 means 100% of desired turn rate
	 */
	void TurnAtRate(float Rate);

	/**
	 * Called via input to turn look up/down at a given rate. 
	 * @param Rate	This is a normalized rate, i.e. 1.0 means 100% of desired turn rate
	 */
	void LookUpAtRate(float Rate);

	/** Handler for when a touch input begins. */
	void TouchStarted(ETouchIndex::Type FingerIndex, FVector Location);

	/** Handler for when a touch input stops. */
	void TouchStopped(ETouchIndex::Type FingerIndex, FVector Location);

protected:

	float damage = 0.0f;

	UPROPERTY(VisibleAnywhere, Replicated, BlueprintReadWrite, Category = "Status", Meta = (ExposeOnSpawn = "true"))
	FName TeamName;

	//Assign data table from bp 
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	class UDataTable* ActionTable;

	UPROPERTY(EditAnywhere, Replicated, BlueprintReadWrite, Category = "Status")
	bool IsHit;

	UPROPERTY(EditAnywhere, Replicated, BlueprintReadWrite, Category = "Anim")
	bool InRagdoll;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Damage")
	bool DoOnce = false;

	UPROPERTY(Replicated, VisibleAnywhere, BlueprintReadWrite, Category = "HitReaction")
	FVector HitLocation;

	UPROPERTY(Replicated, EditAnywhere, BlueprintReadWrite, Category = "HitReaction")
	FName BoneName = "pelvis";

	UPROPERTY(VisibleAnywhere, Replicated, BlueprintReadWrite, Category = "Status")
	float Health;

	UPROPERTY(VisibleAnywhere, Replicated, BlueprintReadWrite, Category = "Status")
	float Stamina;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Input")
	bool bAttacking = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Input")
	bool bComboAttack = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "ActionSkill")
	int32 AttackSectionUUID = 0;

	UPROPERTY(VisibleAnywhere, Replicated, BlueprintReadWrite, Category = "ActionSkill")
	FName AttackSection = "NormalAttack01";

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "ActionSkill")
	float AttackSectionLength = 0.0f;

protected:
	// APawn interface
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	// End of APawn interface

	//Skill sent to server
	UFUNCTION(Reliable, Server, WithValidation, BlueprintCallable, Category = "HitReaction")
	void FireTrace(FVector StartPoint, FVector EndPoint);

	UFUNCTION(Reliable, Server, WithValidation, Category = "HitReaction")
	void DoDamage(AActor* HitActor);

	//Get skills from input touch combo
	UFUNCTION(BlueprintCallable, Category = "ActionSkill")
	void GetButtonSkillAction(FKey Currkeys);

	//Skill sent to server
	UFUNCTION(Reliable, Server, WithValidation, Category = "ActionSkill")
	void ServerExecuteAction(FActionSkill SelectedRow, FName MontageSection);

	//Skill replicate on all client
	UFUNCTION(Reliable, NetMulticast, WithValidation, Category = "ActionSkill")
	void MulticastExecuteAction(FActionSkill SelectedRow, FName MontageSection);

	//Get skills from input touch combo
	UFUNCTION(BlueprintCallable, Category = "ActionSkill")
	void AttackCombo(FActionSkill SelectedRow);



public:
	/** Returns CameraBoom subobject **/
	FORCEINLINE class USpringArmComponent* GetCameraBoom() const { return CameraBoom; }
	/** Returns FollowCamera subobject **/
	FORCEINLINE class UCameraComponent* GetFollowCamera() const { return FollowCamera; }
};

