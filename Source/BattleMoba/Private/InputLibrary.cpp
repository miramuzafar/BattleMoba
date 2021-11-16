// Fill out your copyright notice in the Description page of Project Settings.


#include "InputLibrary.h"
#include "Engine.h"
#include "Math/UnrealMathUtility.h"
#include "Kismet/KismetTextLibrary.h"
#include "Components/WidgetComponent.h"
#include "DrawDebugHelpers.h"

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

//Pass a copy of the actor array to sort and the "central" actor to measure distance from, output a reference to the sorted struct array created in the function
void UInputLibrary::Distance_Sort(UPARAM()TArray<AActor*> Array_To_Sort, UPARAM()AActor * From_Actor, bool Descending, TArray<FActor_Dist>& Sorted_Array)
{
	if (Array_To_Sort.Num() > 0)			// check that input array is populated
	{
		TArray <FActor_Dist> Combined_Data;	// define temporary array of custom struct variables
		FActor_Dist CurActor;
		FVector location1 = From_Actor->GetActorLocation();	// get the world location of the "central" actor distance will be measured from
		float length;


		for (int x = 0; x < Array_To_Sort.Num(); ++x)		// loop through all actors in array
		{
			FVector location2 = Array_To_Sort[x]->GetActorLocation();	// get world location of actor in array
			length = (location1 - location2).Size();					// get distance between "central" actor and this actor from the array
			CurActor.distance = length;									// set the custom struct variable for distance
			CurActor.actor = Array_To_Sort[x];							// set the custom struct actor 

			Combined_Data.Add(CurActor);								// add the newly created struct to the temporary array
		}

		for (FActor_Dist x : Combined_Data)								// using the distance value of each struct order the actors based on distance from "central" actor
		{
			int32 m = Combined_Data.Num();								// run basic bubble sort algorithm
			int32 a, k;
			bool bDidSwap;

			for (a = 0; a < (m - 1); a++)
			{
				bDidSwap = false;

				if (Descending == true)									// sort high to low
				{
					for (k = 0; k < m - a - 1; k++)
					{
						if (Combined_Data[k].distance < Combined_Data[k + 1].distance)
						{
							FActor_Dist z;
							z = Combined_Data[k];
							Combined_Data[k] = Combined_Data[k + 1];
							Combined_Data[k + 1] = z;
							bDidSwap = true;
						}
					}

					if (bDidSwap == false)
					{
						break;
					}
				}
				else													// sort low to high
				{
					for (k = 0; k < m - a - 1; k++)
					{
						if (Combined_Data[k].distance > Combined_Data[k + 1].distance)
						{
							FActor_Dist z;
							z = Combined_Data[k];
							Combined_Data[k] = Combined_Data[k + 1];
							Combined_Data[k + 1] = z;
							bDidSwap = true;
						}
					}

					if (bDidSwap == false)
					{
						break;
					}
				}
			}

			Sorted_Array = Combined_Data;								// set output struct array to sorted temporary array

		}
	}
	else																// if no elements in array exit function 
		;
}

void UInputLibrary::SetUIVisibility(UWidgetComponent* widget, AActor* FromActor)
{
	//only run on client and server
	if (FromActor->GetNetMode() != ENetMode::NM_DedicatedServer)
	{
		FHitResult Hit(ForceInit);

		if (UGameplayStatics::GetPlayerCameraManager(FromActor, 0))
		{
			FVector start = widget->GetComponentLocation();
			FVector End = UGameplayStatics::GetPlayerCameraManager(FromActor, 0)->GetCameraLocation();
			FCollisionQueryParams CollisionParams;
			CollisionParams.AddIgnoredActor(FromActor);

			//DrawDebugBox(FromActor->GetWorld(), widget->GetComponentLocation(), FVector(widget->GetCurrentDrawSize().X / 10.0f, widget->GetCurrentDrawSize().Y / 10.0f, (widget->GetCurrentDrawSize().Y / 10.0f) / 4.0f), FColor::Magenta);

			//Set box collision size
			FCollisionShape BoxCol = FCollisionShape::MakeBox(FVector(widget->GetCurrentDrawSize().X/10.0f, widget->GetCurrentDrawSize().Y/10.0f,(widget->GetCurrentDrawSize().Y/10.0f)/4.0f));

			if (FromActor->GetWorld()->SweepSingleByChannel(Hit, start, End, FQuat::Identity, ECC_Visibility, BoxCol, CollisionParams))
			{

				widget->SetVisibility(false);
			}
			else
			{
				widget->SetVisibility(true);
			}

			//if (FromActor->GetWorld()->LineTraceSingleByChannel(Hit, start, End, ECC_Visibility, CollisionParams) && (Hit.Distance > 100.0f))
			//{
			//	widget->SetVisibility(false);
			//	//widget->GetUserWidgetObject()->SetVisibility(ESlateVisibility::Hidden);
			//}
			//else
			//{
			//	widget->SetVisibility(true);
			//	//widget->GetUserWidgetObject()->SetVisibility(ESlateVisibility::HitTestInvisible);
			//}
		}
	}
}

bool UInputLibrary::PointOnLeftHalfOfScreen(FVector2D Point)
{
	const FVector2D ViewportSize = FVector2D(GEngine->GameViewport->Viewport->GetSizeXY());
	const FVector2D ViewportCenter = FVector2D(ViewportSize.X / 2, ViewportSize.Y / 2);

	return (Point.X <= ViewportCenter.X);
}

void UInputLibrary::AbsoluteValueOfTwoVectors(FVector2D StartValue, FVector2D EndValue, float& x, float& y, float& AbsX, float& AbsY)
{
	FVector2D Total = StartValue - EndValue;

	x = Total.X;
	y = Total.Y;

	AbsX = FGenericPlatformMath::Abs(Total.X);
	AbsY = FGenericPlatformMath::Abs(Total.Y);
}

bool UInputLibrary::DetectLinearSwipe(FVector2D Line1Start, FVector2D Line1End, EInputType& Branches, bool Dos)
{
	//if (!Dos)
	//{
	//	float x;
	//	float y;

	//	float AbsX;
	//	float AbsY;

	//	//Subtract start vector with current vector
	//	FVector2D SubVect = Line1End - Line1Start;

	//	UInputLibrary::AbsoluteValueOfTwoVectors(Line1Start, Line1End, x, y, AbsX, AbsY);

	//	if (SubVect.Size() > 50.0f)
	//	{
	//		//get angle 
	//		float TotalAngle = UKismetMathLibrary::DegAtan2(SubVect.X, SubVect.Y);

	//		UE_LOG(LogTemp, Warning, TEXT("Total Angle : %f"), TotalAngle);

	//		if (AbsX > AbsY)
	//		{
	//			if (x > 50.0f)
	//			{
	//				if (UKismetMathLibrary::InRange_FloatFloat(TotalAngle, -157.5f, -112.5f))
	//				{
	//					Branches = EInputType::UpLeft;
	//					UE_LOG(LogTemp, Warning, TEXT("Up Left"));
	//					GEngine->AddOnScreenDebugMessage(-1, 10.0f, FColor::Emerald, FString::Printf(TEXT("%s"), *GETENUMSTRING("EInputType", Branches)));
	//					return true;
	//				}
	//				if (UKismetMathLibrary::InRange_FloatFloat(TotalAngle, -67.5f, -22.5f))
	//				{
	//					Branches = EInputType::DownLeft;
	//					UE_LOG(LogTemp, Warning, TEXT("Down Left"));
	//					GEngine->AddOnScreenDebugMessage(-1, 10.0f, FColor::Emerald, FString::Printf(TEXT("%s"), *GETENUMSTRING("EInputType", Branches)));
	//					return true;
	//				}
	//				else
	//				{
	//					Branches = EInputType::Left;
	//					UE_LOG(LogTemp, Warning, TEXT("Left"));
	//					GEngine->AddOnScreenDebugMessage(-1, 10.0f, FColor::Emerald, FString::Printf(TEXT("%s"), *GETENUMSTRING("EInputType", Branches)));
	//					return true;
	//				}
	//			}
	//			else if (x < -50.0f)
	//			{
	//				if (UKismetMathLibrary::InRange_FloatFloat(TotalAngle, 112.5f, 157.5f))
	//				{
	//					Branches = EInputType::UpRight;
	//					UE_LOG(LogTemp, Warning, TEXT("Up Right"));
	//					GEngine->AddOnScreenDebugMessage(-1, 10.0f, FColor::Emerald, FString::Printf(TEXT("%s"), *GETENUMSTRING("EInputType", Branches)));
	//					return true;
	//				}
	//				else if (UKismetMathLibrary::InRange_FloatFloat(TotalAngle, 22.5f, 67.5f))
	//				{
	//					Branches = EInputType::DownRight;
	//					UE_LOG(LogTemp, Warning, TEXT("Down Right"));
	//					GEngine->AddOnScreenDebugMessage(-1, 10.0f, FColor::Emerald, FString::Printf(TEXT("%s"), *GETENUMSTRING("EInputType", Branches)));
	//					return true;
	//				}
	//				else
	//				{
	//					Branches = EInputType::Right;
	//					UE_LOG(LogTemp, Warning, TEXT("Right"));
	//					GEngine->AddOnScreenDebugMessage(-1, 10.0f, FColor::Emerald, FString::Printf(TEXT("%s"), *GETENUMSTRING("EInputType", Branches)));
	//					return true;
	//				}
	//			}
	//		}
	//		if (AbsY > AbsX)
	//		{
	//			if (y > 50.0f)
	//			{
	//				if (UKismetMathLibrary::InRange_FloatFloat(TotalAngle, -157.5f, -112.5f))
	//				{
	//					Branches = EInputType::UpLeft;
	//					UE_LOG(LogTemp, Warning, TEXT("Up Left"));
	//					GEngine->AddOnScreenDebugMessage(-1, 10.0f, FColor::Emerald, FString::Printf(TEXT("%s"), *GETENUMSTRING("EInputType", Branches)));
	//					return true;
	//				}
	//				else if (UKismetMathLibrary::InRange_FloatFloat(TotalAngle, 112.5f, 157.5f))
	//				{
	//					Branches = EInputType::UpRight;
	//					UE_LOG(LogTemp, Warning, TEXT("Up Right"));
	//					GEngine->AddOnScreenDebugMessage(-1, 10.0f, FColor::Emerald, FString::Printf(TEXT("%s"), *GETENUMSTRING("EInputType", Branches)));
	//					return true;
	//				}
	//				else
	//				{
	//					Branches = EInputType::Up;
	//					UE_LOG(LogTemp, Warning, TEXT("Up"));
	//					GEngine->AddOnScreenDebugMessage(-1, 10.0f, FColor::Emerald, FString::Printf(TEXT("%s"), *GETENUMSTRING("EInputType", Branches)));
	//					return true;
	//				}
	//			}
	//			else if (y < -50.0f)
	//			{
	//				if (UKismetMathLibrary::InRange_FloatFloat(TotalAngle, -67.5f, -22.5f))
	//				{
	//					Branches = EInputType::DownLeft;
	//					UE_LOG(LogTemp, Warning, TEXT("Down Left"));
	//					GEngine->AddOnScreenDebugMessage(-1, 10.0f, FColor::Emerald, FString::Printf(TEXT("%s"), *GETENUMSTRING("EInputType", Branches)));
	//					return true;
	//				}
	//				else if (UKismetMathLibrary::InRange_FloatFloat(TotalAngle, 22.5f, 67.5f))
	//				{
	//					Branches = EInputType::DownRight;
	//					UE_LOG(LogTemp, Warning, TEXT("Down Right"));
	//					GEngine->AddOnScreenDebugMessage(-1, 10.0f, FColor::Emerald, FString::Printf(TEXT("%s"), *GETENUMSTRING("EInputType", Branches)));
	//					return true;
	//				}
	//				else
	//				{
	//					Branches = EInputType::Down;
	//					UE_LOG(LogTemp, Warning, TEXT("Down"));
	//					GEngine->AddOnScreenDebugMessage(-1, 10.0f, FColor::Emerald, FString::Printf(TEXT("%s"), *GETENUMSTRING("EInputType", Branches)));
	//					return true;
	//				}
	//			}
	//		}
	//	}
	//}
	return false;
}
