// Fill out your copyright notice in the Description page of Project Settings.


#include "InputLibrary.h"

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
