// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/TriggerSphere.h"
#include "BattleMobaCTF.generated.h"

class ABattleMobaCharacter;
/**
 * 
 */
UCLASS()
class BATTLEMOBA_API ABattleMobaCTF : public ATriggerSphere
{
	GENERATED_BODY()
		//Replicated Network setup
		void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

public:
	// Sets default values for this actor's properties
	ABattleMobaCTF();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	//overlap begin function
	UFUNCTION()
		void OnOverlapBegin(class AActor* OverlappedActor, class AActor* OtherActor);

	//overlap end function
	UFUNCTION()
		void OnOverlapEnd(class AActor* OverlappedActor, class AActor* OtherActor);

	////overlap begin function
	//UFUNCTION()
	//	void OnOverlapBegin(UPrimitiveComponent * OverlappedActor, AActor * OtherActor, UPrimitiveComponent * OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult & SweepResult);

	////overlap end function
	//UFUNCTION()
	//	void OnOverlapEnd(UPrimitiveComponent * OverlappedActor, AActor * OtherActor, UPrimitiveComponent * OtherComp, int32 OtherBodyIndex);

public:

	float LastOnScreenTime = 0.01f;

	//		3D UI On TriggerBox
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = Components, meta = (AllowPrivateAccess = "true"))
		class UWidgetComponent* W_ValControl;

	UPROPERTY(VisibleAnywhere, ReplicatedUsing = OnRep_Val, BlueprintReadWrite, Category = "Status")
		float valRadiant = 0.0f;

	UPROPERTY(VisibleAnywhere, ReplicatedUsing = OnRep_Val, BlueprintReadWrite, Category = "Status")
		float valDire = 0.0f;

	UFUNCTION()
		void OnRep_Val();

	//		controlling speed constant for each flag point
	UPROPERTY(EditAnywhere, Category = "Status")
		float ConstantSpeed = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Status")
		float ControllingSpeed = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Status")
		float SpeedMultiplier = 0.05f;

	UPROPERTY(VisibleAnywhere, Replicated, BlueprintReadWrite, Category = "Status")
		FName ControllerTeam = "";

	UPROPERTY(VisibleAnywhere, Replicated, BlueprintReadWrite, Category = "Status")
		TArray<AActor*> OverlappedPlayer;

	UPROPERTY(VisibleAnywhere, Replicated, BlueprintReadWrite, Category = "Status")
		ABattleMobaCharacter* ActivePlayer;

	//	how many of Radiant Players inside of Control Flag radius
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Replicated, Category = "Status")
		int RadiantControl = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Replicated, Category = "Status")
		int DireControl = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Replicated, Category = "Status")
		bool isCompleted = false;

	UPROPERTY(VisibleAnywhere, Replicated, BlueprintReadWrite, Category = "Status")
		TArray<AActor*> GiveGoldActors;

	UPROPERTY(Replicated)
		FTimerHandle FlagTimer;

	UPROPERTY(Replicated)
		FTimerHandle GoldTimer;


public:

	void TimerFunction();

	void GoldTimerFunction();

protected:

	//		Flag Mesh
	UPROPERTY(VisibleAnywhere, meta = (AllowPrivateAccess = "true"))
		class UStaticMeshComponent* MeshFlag;

	//		Trigger collision radius
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Status")
		float RadiusSize = 200.0f;

	//		Flag Name
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Status")
		FName PointName = "BaseFlag";
	
};
