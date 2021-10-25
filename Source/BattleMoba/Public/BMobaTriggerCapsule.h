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

	UPROPERTY(VisibleAnywhere)
		class UStaticMeshComponent* Mesh;

public:

	ABMobaTriggerCapsule();

	UFUNCTION(Server, Reliable, WithValidation)
		void ChangeUI(ABattleMobaCharacter* actor);

	UFUNCTION(NetMulticast, Unreliable, WithValidation)
		void ChangeUIMulticast(ABattleMobaCharacter* actor);

	//3D UI On TriggerBox
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = Components, meta = (AllowPrivateAccess = "true"))
		class UWidgetComponent* W_Val;

	UPROPERTY(VisibleAnywhere, ReplicatedUsing = OnRep_Val, BlueprintReadWrite, Category = "Status")
		float val = 0.0f;
	UFUNCTION()
		void OnRep_Val();

	UPROPERTY(Replicated)
		FTimerHandle FlagTimer;

	UPROPERTY(Replicated)
		FName TeamName;

protected:

		//Called when the game starts or when spawned
		virtual void BeginPlay() override;

private:

	//overlap begin function
	UFUNCTION()
		void OnOverlapBegin(class AActor* OverlappedActor, class AActor* OtherActor);


	//overlap end function
	UFUNCTION()
		void OnOverlapEnd(class AActor* OverlappedActor, class AActor* OtherActor);
	
};
