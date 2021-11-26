// Fill out your copyright notice in the Description page of Project Settings.


#include "BattleMobaCharacterMovementComp.h"

void UBattleMobaCharacterMovementComp::MoveAutonomous(float ClientTimeStamp, float DeltaTime, uint8 CompressedFlags, const FVector & NewAccel)
{
	Super::MoveAutonomous(ClientTimeStamp, DeltaTime, CompressedFlags, NewAccel);
}
