// Fill out your copyright notice in the Description page of Project Settings.

#include "BattleMobaAnimInstance.h"
#include "BattleMobaCharacter.h"

void UBattleMobaAnimInstance::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UBattleMobaAnimInstance, Speed);
	DOREPLIFETIME(UBattleMobaAnimInstance, bMoving);
	DOREPLIFETIME(UBattleMobaAnimInstance, CanMove);
	DOREPLIFETIME(UBattleMobaAnimInstance, canAttack);
	DOREPLIFETIME(UBattleMobaAnimInstance, isBox);
	DOREPLIFETIME(UBattleMobaAnimInstance, isShao);
}

UBattleMobaAnimInstance::UBattleMobaAnimInstance()
{
	
}

void UBattleMobaAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();

	Owner = TryGetPawnOwner();
}

void UBattleMobaAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);

	/*if (!Owner)
	{
		return;
	}

	else if (Owner->IsA(ABattleMobaCharacter::StaticClass()))
	{
		ABattleMobaCharacter* pc = Cast<ABattleMobaCharacter>(Owner);

		if (pc)
		{
			isBox = pc->switchBox;
			isShao = pc->switchShao;
		}
	}*/
}

