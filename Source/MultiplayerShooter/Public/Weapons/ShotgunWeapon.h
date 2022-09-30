// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Weapons/HitScanWeapon.h"
#include "ShotgunWeapon.generated.h"

UCLASS()
class MULTIPLAYERSHOOTER_API AShotgunWeapon : public AHitScanWeapon
{
	GENERATED_BODY()

private:
	UPROPERTY(EditAnywhere, Category = "Shotgun")
	float NumberOfPellets = 12.f;
	
public:
	void FireShotgun(const TArray<FVector_NetQuantize>& HitTargets);
	void ShotgunTraceEndWithScatter(const FVector_NetQuantize& HitTarget, TArray<FVector_NetQuantize>& HitTargets);
};
