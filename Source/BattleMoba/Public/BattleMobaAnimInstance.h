 // Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "Net/UnrealNetwork.h"
#include "BattleMobaAnimInstance.generated.h"

/**
 * 
 */
UCLASS()
class BATTLEMOBA_API UBattleMobaAnimInstance : public UAnimInstance
{
	GENERATED_BODY()
	
public:

	void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

public:

	UPROPERTY(VisibleAnywhere, Replicated, BlueprintReadWrite)
	bool bMoving = false;

	UPROPERTY(VisibleAnywhere, Replicated, BlueprintReadWrite)
		bool CanMove = true;

	UPROPERTY(VisibleAnywhere, Replicated, BlueprintReadWrite)
		float Speed = 0.0f;

protected:

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		int Switcher = 0;

	virtual void NativeUpdateAnimation(float DeltaSeconds) override;
};

