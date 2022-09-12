// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "NiagaraSystem.h"
#include "Components/RocketMovementComponent.h"
#include "Projectiles/ProjectileBase.h"
#include "ProjectileRocket.generated.h"

UCLASS()
class MULTIPLAYERSHOOTER_API AProjectileRocket : public AProjectileBase
{
	GENERATED_BODY()

protected:
	UPROPERTY(VisibleAnywhere, Category = "Components")
	URocketMovementComponent* RocketMovementComponent;

	UPROPERTY(EditAnywhere, Category = "FX")
	USoundCue* ProjectileLoopSoundFX;

	UPROPERTY()
	UAudioComponent* ProjectileLoopSoundComponent;

	UPROPERTY(EditAnywhere, Category = "FX")
	USoundAttenuation* LoopingSoundAttenuation;
	
protected:
	virtual void OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit) override;
	virtual void BeginPlay() override;
	
public:
	AProjectileRocket();
	virtual void Destroyed() override;
};
