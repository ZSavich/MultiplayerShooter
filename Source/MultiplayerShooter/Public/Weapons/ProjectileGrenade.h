// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Projectiles/ProjectileBase.h"
#include "ProjectileGrenade.generated.h"

UCLASS()
class MULTIPLAYERSHOOTER_API AProjectileGrenade : public AProjectileBase
{
	GENERATED_BODY()

private:
	UPROPERTY(EditAnywhere)
	USoundCue* BounceSoundFX;
	
public:
	AProjectileGrenade();
	virtual void Destroyed() override;

protected:
	virtual void BeginPlay() override;
	void HandleOnBounce(const FHitResult& ImpactResult, const FVector& ImpactVelocity);
};
