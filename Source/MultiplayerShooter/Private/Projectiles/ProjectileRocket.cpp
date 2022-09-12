// Fill out your copyright notice in the Description page of Project Settings.

#include "Projectiles/ProjectileRocket.h"

#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "Components/AudioComponent.h"
#include "Components/BoxComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundCue.h"

AProjectileRocket::AProjectileRocket()
{
	ProjectileMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Rocket Mesh"));
	ProjectileMesh->SetupAttachment(GetRootComponent());
	ProjectileMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	
	RocketMovementComponent = CreateDefaultSubobject<URocketMovementComponent>(TEXT("RocketMovementComponent"));
	RocketMovementComponent->bRotationFollowsVelocity = true;
	RocketMovementComponent->InitialSpeed = 1500.f;
	RocketMovementComponent->MaxSpeed = 1500.f;
	RocketMovementComponent->ProjectileGravityScale = 0.f;
	RocketMovementComponent->SetIsReplicated(true);
}

void AProjectileRocket::Destroyed()
{

}

void AProjectileRocket::BeginPlay()
{
	Super::BeginPlay();
	if (!HasAuthority())
	{
		CollisionBox->OnComponentHit.AddDynamic(this, &AProjectileRocket::OnHit);
	}
	SpawnTrailSystem();
	if (ProjectileLoopSoundFX && LoopingSoundAttenuation)
	{
		ProjectileLoopSoundComponent = UGameplayStatics::SpawnSoundAttached(
			ProjectileLoopSoundFX,
			GetRootComponent(),
			FName(),
			GetActorLocation(),
			EAttachLocation::KeepWorldPosition,
			false,
			1.f,
			1.f,
			0.f,
			LoopingSoundAttenuation,
			(USoundConcurrency*)nullptr,
			false
		);
	}
}

void AProjectileRocket::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp,
                              FVector NormalImpulse, const FHitResult& Hit)
{
	if (OtherActor == GetOwner())
	{
		return;
	}
	ExplodeDamage();
	StartDestroyTimer();
	if (ImpactParticleFX)
	{
		UGameplayStatics::SpawnEmitterAtLocation(
			GetWorld(),
			ImpactParticleFX,
			GetActorTransform()
		);
	}
	if (ImpactSoundFX)
	{
		UGameplayStatics::PlaySoundAtLocation(
			GetWorld(),
			ImpactSoundFX,
			GetActorLocation()
		);
	}
	if (ProjectileMesh)
	{
		ProjectileMesh->SetVisibility(false);
	}
	if (CollisionBox)
	{
		CollisionBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}
	const auto TrailInstanceController = TrailSystemComponent->GetSystemInstanceController();
	if (TrailSystemComponent && TrailInstanceController)
	{
		TrailInstanceController->Deactivate();
	}
	if (ProjectileLoopSoundComponent && ProjectileLoopSoundComponent->IsPlaying())
	{
		ProjectileLoopSoundComponent->Stop();
		ProjectileLoopSoundComponent->DestroyComponent();
	}
}

