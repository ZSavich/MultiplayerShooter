// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Projectiles/ProjectileBase.h"
#include "ProjectileBullet.generated.h"

UCLASS()
class MULTIPLAYERSHOOTER_API AProjectileBullet : public AProjectileBase
{
	GENERATED_BODY()

protected:
	virtual void OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit) override;

public:
	AProjectileBullet();
};
