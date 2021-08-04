// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "InputLibrary.generated.h"

USTRUCT(BlueprintType)
struct FActionSkill : public FTableRowBase
{
	GENERATED_BODY()

	bool isOnCD = false;

	//Get key
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage")
		FKey keys;

	//Anim to be played on key pressed
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SkillSwipe")
		UAnimMontage* SkillMoveset;

	//Damage to be dealt from the action
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage")
		float Damage = 0.0f;

	//For array comparison
	bool operator ==(const FActionSkill &other) const
	{
		if (keys == other.keys)
		{
			//UE_LOG(LogTemp, Warning, TEXT("Key %s is %s and %s type is %s and DetectInputOnPressed is %s"), *KeyInput.ToString(), (KeyInput == other.KeyInput)? TEXT("True"): TEXT("False"), *GETENUMSTRING("EInputType", InputType), (InputType == other.InputType) ? TEXT("True") : TEXT("False"), (DetectInputOnPressed == other.DetectInputOnPressed) ? TEXT("True") : TEXT("False"));
			//GEngine->AddOnScreenDebugMessage(-1, 10.0f, FColor::Blue, FString::Printf(TEXT("Key input is %s and Input type is %s and DetectInputOnPressed is %s"), (KeyInput == other.KeyInput) ? TEXT("True") : TEXT("False"), (InputType == other.InputType) ? TEXT("True") : TEXT("False"), (DetectInputOnPressed == other.DetectInputOnPressed) ? TEXT("True") : TEXT("False")));

			return true;
		}
		else
		{
			//UE_LOG(LogTemp, Warning, TEXT("Key %s is %s and %s type is %s"), *KeyInput.ToString(), (KeyInput == other.KeyInput) ? TEXT("True") : TEXT("False"), *GETENUMSTRING("EInputType", InputType), (InputType == other.InputType) ? TEXT("True") : TEXT("False"));
			//GEngine->AddOnScreenDebugMessage(-1, 10.0f, FColor::Blue, FString::Printf(TEXT("Key input is %s and Input type is %s"), (KeyInput == other.KeyInput) ? TEXT("True") : TEXT("False"), (InputType == other.InputType) ? TEXT("True") : TEXT("False")));
			return false;
		}
	}
};

UCLASS()
class BATTLEMOBA_API UInputLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
	
};
