// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "Net/UnrealNetwork.h"
#include "BattleMobaGameMode.generated.h"

class ABattleMobaCharacter;
class ABattleMobaGameState;
class ABattleMobaPlayerState;
class ABattleMobaPC;
class USkeletalMesh;

UCLASS(minimalapi)
class ABattleMobaGameMode : public AGameModeBase
{
	GENERATED_BODY()

	//Replicated Network setup
		void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

protected:

	//class APlayerStart* PStart;

	UPROPERTY(BlueprintReadWrite, Category = "Game State")
		ABattleMobaGameState* GState;

	/*Blueprint Reference of ThirdPersonCharacter class*/
	UPROPERTY(EditDefaultsOnly, Category = "ActorSpawning")
	TSubclassOf<ABattleMobaCharacter> SpawnedActor;

	//************************Clock***********************//
	UPROPERTY(BlueprintReadWrite, Category = "Clock")
		float CurrentTime = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Clock")
		float EndTime = 900.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Map")
		FString MapName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character")
		TArray<USkeletalMesh*> CharSelections;

	//UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character")
	TArray<USkeletalMesh*> Chars;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Character")
		int32 CharIndex = 0;

protected:

	virtual void BeginPlay() override;

	void StartClock();

	UFUNCTION(Category = "Spawn")
		void SpawnBasedOnTeam(FName TeamName);


public:

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Clock")
		float InitialTimer = 0.0f;

	UPROPERTY(EditAnywhere, Replicated, BlueprintReadWrite, Category = "ActorSpawning")
	TArray<class ABattleMobaPC*> Players;

	UPROPERTY()
		FTimerHandle ClockTimer;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NewPlayer")
		APlayerController* newPlayer;

public:
	ABattleMobaGameMode();

	virtual void PostLogin(APlayerController* NewPlayer) override;

	/*virtual AActor* ChoosePlayerStart_Implementation(AController* Player) override;*/

	UFUNCTION(Reliable, Server, WithValidation, Category = "Respawn")
	void RespawnRequested(APlayerController* playerController, FTransform SpawnTransform);

	UFUNCTION(BlueprintCallable, Category = "Score")
		void PlayerKilled(ABattleMobaPlayerState* victim, ABattleMobaPlayerState* killer, TArray<ABattleMobaPlayerState*> assist);
};



