// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "NiagaraSystem.h"
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
	UPROPERTY(VisibleAnywhere, Category = "Components")
	UParticleSystemComponent* TracerComponent;

	UPROPERTY(EditAnywhere, Category = "FX")
	UParticleSystem* TraceParticleFX;

protected:
	UPROPERTY(VisibleAnywhere, Category = "Projectile")
	UStaticMeshComponent* ProjectileMesh;
	
	UPROPERTY(VisibleAnywhere, Category = "Components")
	UProjectileMovementComponent* ProjectileMovementComponent;

	UPROPERTY(EditAnywhere, Category = "FX")
	UNiagaraSystem* TrailSystem;

	UPROPERTY()
	UNiagaraComponent* TrailSystemComponent;
	
	UPROPERTY(EditAnywhere, Category = "FX")
	UParticleSystem* ImpactParticleFX;

	UPROPERTY(EditAnywhere, Category = "FX")
	USoundCue* ImpactSoundFX;

	UPROPERTY(EditAnywhere, Category = "Properties")
	float DestroyTime = 3.f;

	UPROPERTY(EditAnywhere)
	float DamageInnerRadius = 200.f;

	UPROPERTY(EditAnywhere)
	float DamageOuterRadius = 500.f;
	
	FTimerHandle DestroyTimer;

public:
	UPROPERTY(EditAnywhere)
	UBoxComponent* CollisionBox;

	UPROPERTY(EditAnywhere)
	float InitialSpeed = 15000.f;
	
	UPROPERTY(EditDefaultsOnly, Category = "Projectile Stats")
	float Damage = 20.f;

	UPROPERTY(EditDefaultsOnly, Category = "Projectile Stats")
	float HeadShotDamage = 40.f;

	bool bUseServerSideRewind = false;
	FVector_NetQuantize TraceStart;
	FVector_NetQuantize100 InitialVelocity;
	
public:	
	AProjectileBase();

protected:
	UFUNCTION()
	virtual void OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);

	virtual void BeginPlay() override;
	virtual void Destroyed() override;
	
	void DestroyTimerFinished();

	void SpawnTrailSystem();
	void StartDestroyTimer();
	void ExplodeDamage();
};
