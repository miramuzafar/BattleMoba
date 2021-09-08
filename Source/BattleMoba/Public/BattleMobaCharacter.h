// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/PrimitiveComponent.h"
#include "InputLibrary.h"
#include "GameFramework/Character.h"
#include "BattleMobaAnimInstance.h"
#include "BattleMobaCharacter.generated.h"

UCLASS(config = Game)
class ABattleMobaCharacter : public ACharacter
{
	GENERATED_BODY()

		//Replicated Network setup
		void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	/** Camera boom positioning the camera behind the character */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
		class USpringArmComponent* CameraBoom;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Components, meta = (AllowPrivateAccess = "true"))
		class UArrowComponent* BaseArrow;

	/** Follow camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
		class UCameraComponent* FollowCamera;

	/*3DWidget*/
	
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = Components, meta = (AllowPrivateAccess = "true"))
		class USphereComponent* ViewDistanceCol;

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

	////3D UI On Player's head
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = Components, meta = (AllowPrivateAccess = "true"))
		class UWidgetComponent* W_DamageOutput;

public:
	ABattleMobaCharacter();

	/** Base turn rate, in deg/sec. Other scaling may affect final turn rate. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera)
		float BaseTurnRate;

	UPROPERTY()
	float YawRate = 0.0f;

	/** Base look up/down rate, in deg/sec. Other scaling may affect final rate. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera)
		float BaseLookUpRate;

	UPROPERTY(VisibleAnywhere, Replicated, BlueprintReadWrite, Category = "Status", Meta = (ExposeOnSpawn = "true"))
		FString PlayerName;

	UPROPERTY(VisibleAnywhere, ReplicatedUsing = OnRep_Team, BlueprintReadWrite, Category = "Status", Meta = (ExposeOnSpawn = "true"))
		FName TeamName;

	//Setting up character mesh for player
	UPROPERTY(VisibleAnywhere, Replicated, BlueprintReadWrite, Category = "Status", Meta = (ExposeOnSpawn = "true"))
		USkeletalMesh* CharMesh;

	UFUNCTION()
		void OnRep_Team();

	UFUNCTION(Reliable, NetMulticast, WithValidation, Category = "ReceiveDamage")
		void TowerReceiveDamage(ADestructibleTower* Tower, float DamageApply);


	UPROPERTY(VisibleAnywhere, Replicated, BlueprintReadWrite, Category = "Status")
		TArray<class ABattleMobaPlayerState*> DamageDealers;

	UFUNCTION(BlueprintImplementableEvent, Category = "HUD")
		void UpdateHUD();

protected:

	/** Resets HMD orientation in VR. */
	void OnResetVR();

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

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
		float TraceDistance = 0.0f;

		float damage = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "HitReaction")
		UAnimMontage* HitReactionMoveset;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HitReaction")
		UAnimMontage* EnemyHitReactionMoveset;

	//TimerHandle for removing damage dealer array
		FTimerHandle DealerTimer;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "HUD", Meta = (ExposeOnSpawn = "true"))
		UUserWidget* MainWidget;

	//Assign data table from bp 
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		class UDataTable* ActionTable;

	UPROPERTY(EditAnywhere, Replicated, BlueprintReadWrite, Category = "Status")
		bool IsHit;

	UPROPERTY(EditAnywhere, Replicated, BlueprintReadWrite, Category = "Anim")
		bool InRagdoll;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Damage")
		bool DoOnce = false;

	UPROPERTY(Replicated, VisibleAnywhere, BlueprintReadWrite, Category = "Rotate")
		bool Rotate = false;

	UPROPERTY(Replicated, VisibleAnywhere, BlueprintReadWrite, Category = "Rotate")
		AActor* currentTarget;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Rotate")
		bool test = false;


	UPROPERTY(Replicated, VisibleAnywhere, BlueprintReadWrite, Category = "HitReaction")
		FVector AttackerLocation;

	UPROPERTY(Replicated, VisibleAnywhere, BlueprintReadWrite, Category = "HitReaction")
		FVector HitLocation;

	UPROPERTY(Replicated, EditAnywhere, BlueprintReadWrite, Category = "HitReaction")
		FName BoneName = "pelvis";

	UPROPERTY(VisibleAnywhere, ReplicatedUsing = OnRep_Health, BlueprintReadWrite, Category = "Status")
		float Health;

	UFUNCTION()
		void OnRep_Health();

	UPROPERTY(VisibleAnywhere, Replicated, BlueprintReadWrite, Category = "Status")
		float Stamina;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Input")
		bool bAttacking = false;

	//Damage to be dealt from the action
	UPROPERTY(VisibleAnywhere, Replicated, BlueprintReadWrite, Category = "Target")
		bool TargetHead = false;

		int32 AttackSectionUUID;

	UPROPERTY(VisibleAnywhere, Replicated, BlueprintReadWrite, Category = "ActionSkill")
		FName AttackSection = "NormalAttack01";

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "ActionSkill")
		float AttackSectionLength = 0.0f;

	//*********************Knockout and Respawn***********************************//
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Respawn")
	FTimerHandle RespawnTimer;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Movement")
		bool bEnableMove = true;

	UPROPERTY(VisibleAnywhere, Category = "Anims")
		UBattleMobaAnimInstance* AnimInsta;

	UPROPERTY(VisibleAnywhere, Category = "Destructible")
		ADestructibleTower* TowerActor;

	UPROPERTY(VisibleAnywhere, Category = "HitDetection")
		ABattleMobaCharacter* TracedChar;

protected:
	// APawn interface
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	virtual void BeginPlay() override;
	// End of APawn interface

	virtual void Tick(float DeltaTime) override;

	virtual float TakeDamage(float Damage, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser) override;

	/** called when something enters the sphere component */
	UFUNCTION()
	void OnBeginOverlap(UPrimitiveComponent * OverlappedActor, AActor * OtherActor, UPrimitiveComponent * OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult & SweepResult);

	/** called when something leaves the sphere component */
	UFUNCTION()
	void OnEndOverlap(UPrimitiveComponent * OverlappedActor, AActor * OtherActor, UPrimitiveComponent * OtherComp, int32 OtherBodyIndex);

	UFUNCTION()
	void RotateToTargetSetup();

	UFUNCTION(Reliable, Server, WithValidation, Category = "Transformation")
	void ServerRotateToCameraView(FRotator InRot);

	UFUNCTION(Reliable, NetMulticast, WithValidation, Category = "Transformation")
	void RotateToCameraView(FRotator InRot);

	UFUNCTION(BlueprintImplementableEvent, Category = "Damage")
	void Setup3DWidgetVisibility();

	UFUNCTION(BlueprintCallable, Category = "HUDSetup")
	void SetupWidget();

	//Get skills from input touch combo
	UFUNCTION(BlueprintCallable, Category = "CollisionSetup")
		void OnCombatColl(UCapsuleComponent* CombatColl);

	//Get skills from input touch combo
	UFUNCTION(BlueprintCallable, Category = "CollisionSetup")
		void OffCombatColl(UCapsuleComponent* CombatColl);

	//Skill sent to server
	UFUNCTION(Reliable, Server, WithValidation, BlueprintCallable, Category = "HitReaction")
		void FireTrace(FVector StartPoint, FVector EndPoint, bool Head);

	UFUNCTION(Reliable, Server, WithValidation, Category = "HitReaction")
		void DoDamage(AActor* HitActor);

	UFUNCTION(Reliable, Server, WithValidation, Category = "ReceiveDamage")
		void HitReactionServer(AActor* HitActor, float DamageReceived, UAnimMontage* HitMoveset);

	UFUNCTION(Reliable, NetMulticast, WithValidation, Category = "ReceiveDamage")
		void HitReactionClient(AActor* HitActor, float DamageReceived, UAnimMontage* HitMoveset);

	UFUNCTION()
		void ClearDamageDealers();

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

	//*********************Knockout and Respawn***********************************//
	UFUNCTION(Reliable, Client, WithValidation, Category = "Knockout")
	void RespawnCharacter();
	
	//Resets Movement Mode
	UFUNCTION(BlueprintCallable, Category = "Movement")
		void EnableMovementMode();

	UFUNCTION(BlueprintCallable, NetMulticast, Reliable, WithValidation)
		void SetupStats();

public:
	/** Returns CameraBoom subobject **/
	FORCEINLINE class USpringArmComponent* GetCameraBoom() const { return CameraBoom; }
	/** Returns FollowCamera subobject **/
	FORCEINLINE class UCameraComponent* GetFollowCamera() const { return FollowCamera; }
};