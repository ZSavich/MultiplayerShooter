// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Weapons/WeaponBase.h"
#include "HitScanWeapon.generated.h"

UCLASS()
class MULTIPLAYERSHOOTER_API AHitScanWeapon : public AWeaponBase
{
	GENERATED_BODY()

private:
	UPROPERTY(EditAnywhere, Category = "Weapon Scatter", meta = (EditCondition="bUseScatter"))
	float DistanceToSphere = 800.f;

	UPROPERTY(EditAnywhere, Category = "Weapon Scatter", meta = (EditCondition="bUseScatter"))
	float SphereRadius = 75.f;

	UPROPERTY(EditAnywhere, Category = "Weapon Scatter")
	bool bUseScatter = false;
	
protected:
	UPROPERTY(EditAnywhere, Category = "Weapon")
	float Damage = 15.f;

	UPROPERTY(EditAnywhere, Category = "FX")
	UParticleSystem* ImpactParticleFX;

	UPROPERTY(EditAnywhere, Category = "FX")
	UParticleSystem* BeamParticleFX;

	UPROPERTY(EditAnywhere, Category = "FX")
	UParticleSystem* MuzzleFlashParticleFX;

	UPROPERTY(EditAnywhere, Category = "FX")
	USoundCue* FireSoundFX;

	UPROPERTY(EditAnywhere, Category = "FX")
	USoundCue* HitSoundFX;

protected:
	FVector TraceEndWithScatter(const FVector& TraceStart, const FVector& HitTarget);
	void WeaponTraceHit(const FVector& TraceStart, const FVector& HitTarget, FHitResult& OutHit);
	
public:
	virtual void Fire(const FVector& HitTarget) override;
};
