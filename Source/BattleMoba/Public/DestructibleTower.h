// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "DestructibleTower.generated.h"

UCLASS()
class BATTLEMOBA_API ADestructibleTower : public AActor
{
	GENERATED_BODY()

		//Replicated Network setup
		void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	
public:	
	// Sets default values for this actor's properties
	ADestructibleTower();

	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	
	
protected:
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Destructible)
		class USceneComponent* TriggerComponent;

	/*UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Destructible)
		class UDestructibleComponent* DestructibleComponent;*/

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = UI, meta = (AllowPrivateAccess = "true"))
		class UWidgetComponent* W_Health;

	UPROPERTY(VisibleAnywhere)
		class UStaticMeshComponent* TowerMesh;

	UPROPERTY(EditDefaultsOnly, ReplicatedUsing = OnRep_UpdateHealth, BlueprintReadOnly, Category = Destructible)
		float DamageValue;

	UPROPERTY(EditDefaultsOnly, ReplicatedUsing = OnRep_UpdateHealth, BlueprintReadOnly, Category = Destructible)
		float ImpulseValue;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = UI)
		class UUserWidget* W_DisplayHealth;

	




	UFUNCTION(BlueprintCallable)
		virtual float TakeDamage(float value, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser) override;


	

public:	

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Destructible)
		bool isTriggerEnabled;

	UPROPERTY(VisibleAnywhere, ReplicatedUsing = OnRep_Destroy, BlueprintReadOnly, Category = Destructible)
		bool isDestroyed;

	UFUNCTION()
		void OnRep_Destroy();

	UPROPERTY(EditAnywhere, Replicated, BlueprintReadWrite, Category = Destructible)
		bool IsHit;

	UPROPERTY(EditAnywhere, ReplicatedUsing = OnRep_Team, BlueprintReadOnly, Category = Destructible, Meta = (ExposeOnSpawn = "true"))
		FName TeamName;

	UFUNCTION()
		void OnRep_Team();

	UPROPERTY(EditAnywhere, ReplicatedUsing = OnRep_UpdateHealth, BlueprintReadOnly, Category = Destructible)
		float CurrentHealth;

	UFUNCTION()
		void OnRep_UpdateHealth();

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Destructible)
		float MaxHealth;

	UPROPERTY(EditDefaultsOnly)
		class UMaterialInstanceDynamic* DynamicMaterial;

	// Called every frame
	virtual void Tick(float DeltaTime) override;

};
