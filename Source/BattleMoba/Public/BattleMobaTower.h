// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "BattleMobaTower.generated.h"

UCLASS()
class BATTLEMOBA_API ABattleMobaTower : public AActor
{
	GENERATED_BODY()

		//Replicated Network setup
		void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	
public:	
	// Sets default values for this actor's properties
	ABattleMobaTower();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere)
		class USceneComponent* Root;

	UPROPERTY(VisibleAnywhere)
		class UStaticMeshComponent* TowerMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = Components, meta = (AllowPrivateAccess = "true"))
		class UWidgetComponent* W_Health;

	UPROPERTY(VisibleAnywhere, ReplicatedUsing = OnRep_UpdateHealth, BlueprintReadWrite, Category = "Status")
		float Health;

	UFUNCTION()
		void OnRep_UpdateHealth();

	UPROPERTY(EditAnywhere, Category = "Status")
		float MaxHealth = 500;

	UPROPERTY(VisibleAnywhere)
		class UUserWidget* W_DisplayHealth;



	UFUNCTION(BlueprintCallable)
		virtual float TakeDamage(float value, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser) override;


public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

};
