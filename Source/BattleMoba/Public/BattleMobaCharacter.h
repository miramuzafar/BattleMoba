// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/PrimitiveComponent.h"
#include "InputLibrary.h"
#include "GameFramework/Character.h"
#include "BattleMobaAnimInstance.h"
#include "BattleMobaCharacter.generated.h"

class ABMobaTriggerCapsule;
struct FTimerHandle;
class ABattleMobaCTF;

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

	//Outline
	//Setting up character mesh for player
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = Components, Meta = (AllowPrivateAccess = "true"))
		class UStaticMeshComponent* Outline;
	
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

	UPROPERTY(VisibleAnywhere, Replicated, BlueprintReadWrite, Category = "Status")
		TArray<class ABattleMobaPlayerState*> DamageDealers;

	UPROPERTY(VisibleAnywhere, Replicated, BlueprintReadWrite, Category = "ControlFlag")
		bool CTFentering;

	UPROPERTY(VisibleAnywhere, Replicated, Category = "ControlFlag")
		TArray<AActor*> ActorsToGetGold;

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

	/** Input call camera shake */
	void OnCameraShake();


protected:

	//rain checks on action skills to be executed
	bool ActionEnabled = true;

	//Init Swipe mechanics
	bool IsPressed = false;

	//SwipeToRotate
	bool StartRotate = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Status")
	bool InitRotateToggle = false;

	FVector2D TouchStart;

	FVector2D TouchEnd;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
		float TraceDistance = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Status")
		int OrbsAmount = 0;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Damage")
		float BaseDamage = 0.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Damage")
		int MinDamage = 0;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Damage")
		int MaxDamage = 0;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Damage")
		float ActualDamage = 0.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Damage")
		float BuffDamage = 1.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Damage")
		float Defence = 110.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Damage")
		float BuffDefence = 1.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Damage")
		float ReducedDefence = 0.0f;

	UPROPERTY(VisibleAnywhere, Replicated, Category = "ActionSkill")
		UAnimMontage* CounterMoveset;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "HitReaction")
		UAnimMontage* HitReactionMoveset;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HitReaction")
		UAnimMontage* FrontHitMoveset;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HitReaction")
		UAnimMontage* BackHitMoveset;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HitReaction")
		UAnimMontage* RightHitMoveset;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HitReaction")
		UAnimMontage* LeftHitMoveset;

	//TimerHandle for removing damage dealer array
		FTimerHandle DealerTimer;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "HUD", Meta = (ExposeOnSpawn = "true"))
		UUserWidget* MainWidget;

	//Assign data table from bp 
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		class UDataTable* ActionTable;

	UPROPERTY(EditAnywhere, Replicated, BlueprintReadWrite, Category = "Status")
		bool IsHit;

	UPROPERTY(EditDefaultsOnly, Category = "HitReaction")
		bool IsStunned = false;

	UPROPERTY(EditDefaultsOnly, Category = "HitReaction")
		bool OnSpecialAttack = false;

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

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "UI")
		bool WithinVicinity = false;

	UPROPERTY(BlueprintReadOnly, Category = "Rotate")
		TArray<AActor*> FoundActors;


	UPROPERTY(Replicated, VisibleAnywhere, BlueprintReadWrite, Category = "HitReaction")
		FVector AttackerLocation;

	UPROPERTY(Replicated, VisibleAnywhere, BlueprintReadWrite, Category = "HitReaction")
		FVector HitLocation;

	UPROPERTY(Replicated, EditAnywhere, BlueprintReadWrite, Category = "HitReaction")
		FName BoneName = "pelvis";

	UPROPERTY(VisibleAnywhere, ReplicatedUsing = OnRep_Health, BlueprintReadWrite, Category = "Status")
		float Health;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Status")
		float MaxHealth;

	UFUNCTION()
		void OnRep_Health();

	UPROPERTY(VisibleAnywhere, Replicated, BlueprintReadWrite, Category = "Status")
		float Stamina;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Input")
		bool bAttacking = false;

	//Damage to be dealt from the action
	UPROPERTY(VisibleAnywhere, Replicated, BlueprintReadWrite, Category = "Target")
		bool TargetHead = false;

	UPROPERTY(VisibleAnywhere, Replicated, BlueprintReadWrite, Category = "ActionSkill")
		FName AttackSection = "NormalAttack01";

	UPROPERTY(VisibleAnywhere, Replicated, BlueprintReadWrite, Category = "ActionSkill")
		FName CurrentSection = "NormalAttack01";

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "ActionSkill")
		float AttackSectionLength = 0.0f;

	UPROPERTY(EditDefaultsOnly, Category = "ActionSkill")
		float RemainingLength = 0.0f;

	//*********************Knockout and Respawn***********************************//
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Respawn")
		FTimerHandle RespawnTimer;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Movement")
		bool bEnableMove = true;

	UPROPERTY(VisibleAnywhere, Category = "Anim")
		UBattleMobaAnimInstance* AnimInsta;

	UPROPERTY(VisibleAnywhere, Category = "Destructible")
		ADestructibleTower* TowerActor;

	UPROPERTY(VisibleAnywhere, Category = "HitDetection")
		ABattleMobaCharacter* TracedChar;

	UPROPERTY(Replicated, EditDefaultsOnly, BlueprintReadWrite, Category = "HitReaction")
		UParticleSystem* HitEffect;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "HitReaction")
		FName ActiveSocket;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Replicated, Category = "ControlFlag")
		FName CTFteam = "";

	UPROPERTY(VisibleAnywhere, Category = "Rotate")
		float RotateRadius = 100.0f;

	UPROPERTY(Replicated, VisibleAnywhere, Category = "Rotate")
		class AActor* closestActor;

	UPROPERTY(Replicated, VisibleAnywhere, Category = "Rotate")
		class ABattleMobaCharacter* RotateToActor;

	UPROPERTY(VisibleAnywhere, Category = "Rotate")
		TArray<class AActor*> IgnoreActors;

	UPROPERTY()
	TArray<ABattleMobaCTF*> Towers;

		TEnumAsByte<ETouchIndex::Type> MoveTouchIndex;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
		TEnumAsByte<ETouchIndex::Type> RotTouchIndex;


protected:
	// APawn interface
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	virtual void BeginPlay() override;
	// End of APawn interface

	virtual void Tick(float DeltaTime) override;

	void AddSwipeVectorToMovementInput();

	void AddSwipeVectorToRotationInput();

	virtual float TakeDamage(float Damage, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser) override;

	UFUNCTION(BlueprintCallable, meta = (ExpandEnumAsExecs = Type))
	void CheckSwipeType(EInputType Type, FVector2D Location, TEnumAsByte<ETouchIndex::Type> TouchIndex);

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

	UFUNCTION(BlueprintCallable, Category = "HUDSetup")
	void HideHPBar();

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
		void HitReactionServer(AActor* HitActor, float DamageReceived, UAnimMontage* HitMoveset, FName MontageSection);

	UFUNCTION(Reliable, NetMulticast, WithValidation, Category = "ReceiveDamage")
		void HitReactionClient(AActor* HitActor, float DamageReceived, UAnimMontage* HitMoveset, FName MontageSection);

	UFUNCTION(Reliable, Server, WithValidation, BlueprintCallable, Category = "HitReaction")
		void StunPlayerServer(bool checkStun);

	UFUNCTION(Reliable, NetMulticast, WithValidation, BlueprintCallable, Category = "HitReaction")
		void StunPlayerClient(bool checkStun);

	UFUNCTION(Reliable, Server, WithValidation, BlueprintCallable, Category = "HitReaction")
		void ServerRotateHitActor(AActor* HitActor, AActor* Attacker);

	UFUNCTION(Reliable, NetMulticast, WithValidation, BlueprintCallable, Category = "HitReaction")
		void MulticastRotateHitActor(AActor* HitActor, AActor* Attacker);

	UFUNCTION(Reliable, Server, WithValidation, BlueprintCallable, Category = "HitReaction")
		void ServerSpawnEffect(ABattleMobaCharacter* EmitActor, ABattleMobaCharacter* HitActor);

	UFUNCTION(Reliable, NetMulticast, WithValidation, BlueprintCallable, Category = "HitReaction")
		void MulticastSpawnEffect(ABattleMobaCharacter* EmitActor, ABattleMobaCharacter* HitActor);

	UFUNCTION(Reliable, Server, WithValidation, BlueprintCallable, Category = "HitReaction")
		void SetActiveSocket(FName SocketName);

	UFUNCTION(Reliable, NetMulticast, WithValidation, Category = "HitReaction")
		void MulticastSetActiveSocket(FName SocketName);


	UFUNCTION()
		void ClearDamageDealers();

	//Skill sent to server
	UFUNCTION(Reliable, Server, WithValidation, Category = "ActionSkill")
		void ServerExecuteAction(FActionSkill SelectedRow, FName ActiveSection, FName MontageSection, bool bSpecialAttack);

	//Skill replicate on all client
	UFUNCTION(Reliable, NetMulticast, WithValidation, Category = "ActionSkill")
		void MulticastExecuteAction(FActionSkill SelectedRow, FName ActiveSection, FName MontageSection, bool bSpecialAttack);

	//Get skills from input touch combo
	UFUNCTION(BlueprintCallable, Category = "ActionSkill")
		void AttackCombo(FActionSkill SelectedRow);

	UFUNCTION(Reliable, Server, WithValidation, Category = "ActionSkill")
		void ServerCounterAttack(ABattleMobaCharacter* hitActor);

	UFUNCTION(Reliable, NetMulticast, WithValidation, Category = "ActionSkill")
		void MulticastCounterAttack(ABattleMobaCharacter* hitActor);

	//*********************Knockout and Respawn***********************************//
	UFUNCTION(Reliable, Client, WithValidation, Category = "Knockout")
		void RespawnCharacter();
	
	//Resets Movement Mode
	UFUNCTION(BlueprintCallable, Category = "Movement")
		void EnableMovementMode();

	UFUNCTION(BlueprintCallable, NetMulticast, Reliable, WithValidation)
		void SetupStats();

	UFUNCTION(Server, Reliable, WithValidation)
		void ControlFlagServer(ABattleMobaCTF* cf);

	UFUNCTION(NetMulticast, Unreliable, WithValidation)
		void ControlFlagMulticast(ABattleMobaCTF* cf, FName Team);

	UFUNCTION(Reliable, Server, WithValidation, BlueprintCallable, Category = "ActionSkill")
		void DetectNearestTarget();

	UFUNCTION(Reliable, NetMulticast, WithValidation, BlueprintCallable, Category = "ActionSkill")
		void RotateNearestTarget(AActor* Target);

	UFUNCTION(BlueprintImplementableEvent, Category = "Effects")
		void CombatCamShake();

public:
	/** Returns CameraBoom subobject **/
	FORCEINLINE class USpringArmComponent* GetCameraBoom() const { return CameraBoom; }
	/** Returns FollowCamera subobject **/
	FORCEINLINE class UCameraComponent* GetFollowCamera() const { return FollowCamera; }

	UFUNCTION(BlueprintImplementableEvent, Category = "HUD")
		void UpdateHUD();

	UFUNCTION(BlueprintImplementableEvent, Category = "HUD")
		void CreateCPHUD();

	UFUNCTION(Reliable, NetMulticast, WithValidation, Category = "ReceiveDamage")
		void TowerReceiveDamage(ADestructibleTower* Tower, float DamageApply);

	/*******************SAFEZONE*****************************************/

	void SafeZone(ABMobaTriggerCapsule* TriggerZone);

	UFUNCTION(Server, Reliable, WithValidation)
		void SafeZoneServer(ABMobaTriggerCapsule* TriggerZone);

	UFUNCTION(NetMulticast, Unreliable, WithValidation)
		void SafeZoneMulticast(ABMobaTriggerCapsule* TriggerZone);

	/***********************CONTROL FLAG MODE*****************************/
	void ControlFlagMode(ABattleMobaCTF* cf);

	//Get skills from input touch combo
	UFUNCTION(BlueprintCallable, Category = "ActionSkill")
		void GetButtonSkillAction(FKey Currkeys, FString ButtonName, bool& cooldown, float& CooldownVal);

};