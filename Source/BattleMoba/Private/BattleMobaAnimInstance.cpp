// Fill out your copyright notice in the Description page of Project Settings.

#include "BattleMobaAnimInstance.h"

void UBattleMobaAnimInstance::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UBattleMobaAnimInstance, Speed);
	DOREPLIFETIME(UBattleMobaAnimInstance, bMoving);
}

