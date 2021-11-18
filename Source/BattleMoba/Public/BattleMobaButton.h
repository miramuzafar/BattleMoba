// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/Button.h"
#include "BattleMobaButton.generated.h"

//Event binders
//DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FLoadDelegate, FString, name);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FClickDelegate);

UCLASS()
class BATTLEMOBA_API UBattleMobaButton : public UButton
{
	GENERATED_BODY()

public:
	UBattleMobaButton();

	/*UPROPERTY()
		FLoadDelegate load;*/

	UPROPERTY()
		FClickDelegate click;

	UFUNCTION()
	void OnPress();

	UFUNCTION(BlueprintImplementableEvent, Category = "ActionSkill")
	void LoadSkillFile(const FString &name);
	//virtual void LoadSkillFile_Implementation(const FString &name);
};
