// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "InputLibrary.generated.h"

class UWidgetComponent;

USTRUCT(BlueprintType)
struct FActionSkill : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY()
		bool isOnCD = false;

	//If cooldown mechanic is applied
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cooldown")
		bool IsUsingCD = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cooldown", meta = (EditCondition = "IsUsingCD"))
		float CDDuration = 0.0f;

	//If translation mechanic is applied
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Translate Player")
		bool UseTranslate = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Translate Player", meta = (EditCondition = "UseTranslate"))
		float TranslateDist;

	//If cooldown mechanic is applied
	//If combo mechanic is applied
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UseSection")
		bool UseSection = false;

	//How many sections are used in this montage
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Section", meta = (EditCondition = "UseSection"))
		int Section = 0;

	//Get key
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage")
		FKey keys;

	//Anim to be played on key pressed
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Anim")
		UAnimMontage* SkillMoveset;

	//Damage to be dealt from the action
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage")
		int MinDamage = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage")
		int MaxDamage = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effect")
		UParticleSystem* HitImpact;

	//Anim to be played on hit detection
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Anim")
		UAnimMontage* HitMoveset;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Anim")
		UAnimMontage* FrontHitMoveset;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Anim")
		UAnimMontage* BackHitMoveset;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Anim")
		UAnimMontage* LeftHitMoveset;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Anim")
		UAnimMontage* RightHitMoveset;

	//Check if target hit is head
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Target")
		bool TargetIsHead = false;

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

USTRUCT(BlueprintType)      // Define custom struct and expose to blueprints
struct FActor_Dist          // This will be the name of the new struct in BPs 
{
	GENERATED_BODY()

public:

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Actor Distance")    // allow this property to be edited in BPs
		float distance = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Actor Distance")    // allow this property to be edited in BPs
		AActor* actor = NULL;

};

UCLASS()
class BATTLEMOBA_API UInputLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
//
//	UFUNCTION(BlueprintPure, Category = "Rotation")
//	static FRotator LookRotation(FVector lookAt, FVector upDirection);

	UFUNCTION(BlueprintCallable, Category = "DateAndTime")
		static FDateTime GetCurrentDateAndTime();

	UFUNCTION(BlueprintCallable, Category = "DateAndTime")
		static float GetCurrentTimeInMinute();

	UFUNCTION(BlueprintCallable, Category = "DateAndTime")
		static FTimespan GetCurrentTime();

	UFUNCTION(BlueprintPure, Category = "DateAndTime")
		static FString DisplayMinutesSecondsFormat(float Seconds);

	UFUNCTION(BlueprintCallable, Category = "Sorting Functions")    // Sort actors by distance from target actor    
		static void  Distance_Sort(UPARAM() TArray <AActor*> Array_To_Sort, UPARAM()AActor* From_Actor, bool Descending, TArray <FActor_Dist> &Sorted_Array);

	UFUNCTION(BlueprintCallable, Category = "Displays") //Hide 3D widget behind building etc
		static void SetUIVisibility(UWidgetComponent* widget, AActor* FromActor);
};