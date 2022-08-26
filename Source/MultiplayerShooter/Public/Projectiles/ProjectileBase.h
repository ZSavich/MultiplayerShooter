// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ProjectileBase.generated.h"

class UBoxComponent;
class UProjectileMovementComponent;
class USoundCue;

UCLASS()
class MULTIPLAYERSHOOTER_API AProjectileBase : public AActor
{
	GENERATED_BODY()

private:
	UPROPERTY(EditAnywhere)
	UBoxComponent* CollisionBox;

	UPROPERTY(VisibleAnywhere, Category = "Components")
	UProjectileMovementComponent* ProjectileMovementComponent;

	UPROPERTY(VisibleAnywhere, Category = "Components")
	UParticleSystemComponent* TracerComponent;

	UPROPERTY(EditAnywhere, Category = "FX")
	UParticleSystem* TraceParticleFX;

	UPROPERTY(EditAnywhere, Category = "FX")
	UParticleSystem* ImpactParticleFX;

	UPROPERTY(EditAnywhere, Category = "FX")
	USoundCue* ImpactSoundFX;
	
public:	
	AProjectileBase();

protected:
	UPROPERTY(EditDefaultsOnly, Category = "Projectile Stats")
	float Damage = 20.f;
	
	virtual void BeginPlay() override;
	virtual void Destroyed() override;

	UFUNCTION()
	virtual void OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);
};
