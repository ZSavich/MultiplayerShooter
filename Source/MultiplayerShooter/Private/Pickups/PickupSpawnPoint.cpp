// Fill out your copyright notice in the Description page of Project Settings.

#include "Pickups/PickupSpawnPoint.h"

APickupSpawnPoint::APickupSpawnPoint()
{
 	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;
}

void APickupSpawnPoint::BeginPlay()
{
	Super::BeginPlay();
	StartSpawnPickupTimer(nullptr);
}

void APickupSpawnPoint::StartSpawnPickupTimer(AActor* DestroyedActor)
{
	const float SpawnTime = FMath::FRandRange(SpawnPickupTimeMin, SpawnPickupTimeMax);
	GetWorldTimerManager().SetTimer(SpawnPickupTimer, this, &APickupSpawnPoint::SpawnPickupTimerFinished, SpawnTime);
}

void APickupSpawnPoint::SpawnPickup()
{
	const int32 NumPickupClasses = PickupClasses.Num();
	if (NumPickupClasses > 0.f)
	{
		const int32 Selection = FMath::RandRange(0, NumPickupClasses - 1);
		SpawnedPickup = GetWorld()->SpawnActor<APickupBase>(PickupClasses[Selection], GetActorTransform());
		if (HasAuthority())
		{
			SpawnedPickup->OnDestroyed.AddDynamic(this, &APickupSpawnPoint::StartSpawnPickupTimer);
		}
	}
}

void APickupSpawnPoint::SpawnPickupTimerFinished()
{
	if (HasAuthority())
	{
		SpawnPickup();
	}
}
