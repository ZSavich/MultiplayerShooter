// Fill out your copyright notice in the Description page of Project Settings.

#include "Projectiles/ProjectileBase.h"

#include "Character/BlasterCharacter.h"
#include "Components/BoxComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "MultiplayerShooter/MultiplayerShooter.h"
#include "Sound/SoundCue.h"

AProjectileBase::AProjectileBase()
{
 	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;

	CollisionBox = CreateDefaultSubobject<UBoxComponent>(TEXT("CollisionBox"));
	SetRootComponent(CollisionBox);
	CollisionBox->SetCollisionObjectType(ECC_WorldDynamic);
	CollisionBox->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	CollisionBox->SetCollisionResponseToAllChannels(ECR_Ignore);
	CollisionBox->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);
	CollisionBox->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Block);
	CollisionBox->SetCollisionResponseToChannel(ECC_SkeletalMesh, ECR_Block);

	ProjectileMovementComponent = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovementComponent"));
	ProjectileMovementComponent->bRotationFollowsVelocity = true;
	ProjectileMovementComponent->InitialSpeed = 15000.f;
	ProjectileMovementComponent->MaxSpeed = 15000.f;
}

void AProjectileBase::BeginPlay()
{
	Super::BeginPlay();

	if (TraceParticleFX)
	{
		TracerComponent = UGameplayStatics::SpawnEmitterAttached(
			TraceParticleFX,
			CollisionBox,
			FName(),
			GetActorLocation(),
			GetActorRotation(),
			EAttachLocation::KeepWorldPosition
		);
	}
	if (HasAuthority())
	{
		CollisionBox->OnComponentHit.AddDynamic(this, &AProjectileBase::OnHit);
	}
}

void AProjectileBase::Destroyed()
{
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
	Super::Destroyed();
}

void AProjectileBase::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp,
                            FVector NormalImpulse, const FHitResult& Hit)
{
	Destroy();
}
