// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/TriggerCapsule.h"
#include "BMobaTriggerCapsule.generated.h"

/**
 * 
 */
UCLASS()
class BATTLEMOBA_API ABMobaTriggerCapsule : public ATriggerCapsule
{
	GENERATED_BODY()

	//Replicated Network setup
		void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	//3D UI On TriggerBox
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = Components, meta = (AllowPrivateAccess = "true"))
		class UWidgetComponent* W_Val;

public:

	ABMobaTriggerCapsule();

	UPROPERTY(VisibleAnywhere, ReplicatedUsing = OnRep_Val, BlueprintReadWrite, Category = "Status")
		float val = 0.0f;
	UFUNCTION()
		void OnRep_Val();

protected:
	//Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UPROPERTY()
		FTimerHandle FlagTimer;

private:

	//overlap begin function
	UFUNCTION()
		void OnOverlapBegin(class AActor* OverlappedActor, class AActor* OtherActor);

	//overlap end function
	UFUNCTION()
		void OnOverlapEnd(class AActor* OverlappedActor, class AActor* OtherActor);
	
};
