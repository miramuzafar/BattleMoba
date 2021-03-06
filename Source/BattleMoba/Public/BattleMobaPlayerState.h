// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "BattleMobaPlayerState.generated.h"

class USkeletalMesh;
/**
 * 
 */
UCLASS()
class BATTLEMOBA_API ABattleMobaPlayerState : public APlayerState
{
	GENERATED_BODY()

	void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const;
	
public:

	UPROPERTY(VisibleAnywhere, Replicated, BlueprintReadWrite, Category = "Status")
	float MaxHealth = 750.0f;

	UPROPERTY(VisibleAnywhere, Replicated, BlueprintReadWrite, Category = "Status")
	FName TeamName;

	UPROPERTY(VisibleAnywhere, Replicated, BlueprintReadWrite, Category = "Status")
	int32 Pi = 0;

	UPROPERTY(VisibleAnywhere, Replicated, BlueprintReadWrite, Category = "Status")
	int Kill = 0;

	UPROPERTY(VisibleAnywhere, Replicated, BlueprintReadWrite, Category = "Status")
	int Death = 0;

	UPROPERTY(VisibleAnywhere, Replicated, BlueprintReadWrite, Category = "Status")
	int Assist = 0;

	UPROPERTY(VisibleAnywhere, Replicated, BlueprintReadWrite, Category = "Status")
	USkeletalMesh* CharMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Respawn")
	FTransform SpawnTransform;

	UPROPERTY(VisibleAnywhere, Replicated, BlueprintReadWrite, Category = "Item")
		int ChiOrbs = 0;

	UPROPERTY(VisibleAnywhere, Replicated, BlueprintReadWrite, Category = "Item")
		int RespawnTimeCounter = 30;

	UPROPERTY(VisibleAnywhere, Replicated, BlueprintReadWrite, Category = "Respawn")
	FTimerHandle RespawnHandle;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		class UDataTable* ActionTable;

public:
	UFUNCTION(Reliable, Client, WithValidation, Category = "PI")
		void SetPlayerIndex(int32 PlayerIndex);

	//For displaying respawn time count
	UFUNCTION(BlueprintImplementableEvent, Category = "Damage")
		void DisplayRespawnTime();
};
