// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "BattleMobaCharacterMovementComp.generated.h"

/**
 * 
 */
UCLASS()
class BATTLEMOBA_API UBattleMobaCharacterMovementComp : public UCharacterMovementComponent
{
	GENERATED_BODY()

	virtual void MoveAutonomous(float ClientTimeStamp, float DeltaTime, uint8 CompressedFlags, const FVector & NewAccel) override;
	
};
