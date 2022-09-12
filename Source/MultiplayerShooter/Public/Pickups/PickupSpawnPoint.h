// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PickupBase.h"
#include "GameFramework/Actor.h"
#include "PickupSpawnPoint.generated.h"

UCLASS()
class MULTIPLAYERSHOOTER_API APickupSpawnPoint : public AActor
{
	GENERATED_BODY()

protected:
	UPROPERTY(EditAnywhere)
	TArray<TSubclassOf<APickupBase>> PickupClasses;

	UPROPERTY(Transient)
	APickupBase* SpawnedPickup;

private:
	FTimerHandle SpawnPickupTimer;

	UPROPERTY(EditAnywhere)
	float SpawnPickupTimeMin = 5.f;

	UPROPERTY(EditAnywhere)
	float SpawnPickupTimeMax = 10.f;
	
public:	
	APickupSpawnPoint();

protected:
	virtual void BeginPlay() override;

	UFUNCTION()
	void StartSpawnPickupTimer(AActor* DestroyedActor);
	
	void SpawnPickup();
	void SpawnPickupTimerFinished();

};
