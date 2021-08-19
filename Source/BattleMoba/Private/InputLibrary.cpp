// Fill out your copyright notice in the Description page of Project Settings.


#include "InputLibrary.h"
#include "Math/UnrealMathUtility.h"
#include "Kismet/KismetTextLibrary.h"

FDateTime UInputLibrary::GetCurrentDateAndTime()
{
	return FDateTime::Now();
}

float UInputLibrary::GetCurrentTimeInMinute()
{
	return GetCurrentTime().GetTotalMinutes();
}

FTimespan UInputLibrary::GetCurrentTime()
{
	return GetCurrentDateAndTime().GetTimeOfDay();
}

FString UInputLibrary::DisplayMinutesSecondsFormat(float Seconds)
{
	int newDivision = FMath::FloorToInt(Seconds/60.0f);
	int newModulo = FMath::FloorToInt(FMath::Fmod(Seconds, 60.0f));

	//Converts to string
	FString DtoStr = FString::FromInt(newDivision);
	FString MtoStr = FString::FromInt(newModulo);

	/*return string will produce result if newdivision is larger than 9 or vice versa*/
	FString DString = newDivision > 9 ? "0" + DtoStr : DtoStr;

	/*return string will produce result if newdivision is larger than 9 or vice versa*/
	FString DMod = newModulo > 9 ? MtoStr : MtoStr;

	//Set modulo result to two digit formats
	FText newModT = UKismetTextLibrary::Conv_IntToText(FCString::Atoi(*DMod), false, true, 2);

	return DString + ":"+ newModT.ToString();
}
