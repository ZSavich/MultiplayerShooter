// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Weapons/WeaponBase.h"
#include "ProjectileWeapon.generated.h"

class AProjectileBase;

UCLASS()
class MULTIPLAYERSHOOTER_API AProjectileWeapon : public AWeaponBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere)
	TSubclassOf<AProjectileBase> ProjectileClass;

public:
	virtual void Fire(const FVector& HitTarget) override;
};
