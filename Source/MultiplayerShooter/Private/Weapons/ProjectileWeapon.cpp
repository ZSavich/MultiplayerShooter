// Fill out your copyright notice in the Description page of Project Settings.

#include "Weapons/ProjectileWeapon.h"

#include "Engine/SkeletalMeshSocket.h"
#include "Projectiles/ProjectileBase.h"

void AProjectileWeapon::Fire(const FVector& HitTarget)
{
	Super::Fire(HitTarget);
	
	APawn* InstigatorPawn = Cast<APawn>(GetOwner());
	const USkeletalMeshSocket* MuzzleFlashSocket = GetWeaponMesh()->GetSocketByName(TEXT("MuzzleFlash"));
	if (InstigatorPawn && MuzzleFlashSocket)
	{
		const FTransform SocketTransform = MuzzleFlashSocket->GetSocketTransform(GetWeaponMesh());
		const FVector ToTarget = HitTarget - SocketTransform.GetLocation();
		const FRotator TargetRotation = ToTarget.Rotation();
		UWorld* World = GetWorld();

		FActorSpawnParameters SpawnParams;
		SpawnParams.Owner = GetOwner();
		SpawnParams.Instigator = InstigatorPawn;

		AProjectileBase* SpawnerProjectile = nullptr;

		if (World)
		{
			if (bUseServerSideRewind)
			{
				if (InstigatorPawn->HasAuthority())
				{
					if (InstigatorPawn->IsLocallyControlled())
					{
						if (ProjectileClass)
						{
							SpawnerProjectile = World->SpawnActor<AProjectileBase>(ProjectileClass, SocketTransform.GetLocation(), TargetRotation, SpawnParams);
						}
						SpawnerProjectile->bUseServerSideRewind = false;
						SpawnerProjectile->Damage = Damage;
						SpawnerProjectile->HeadShotDamage = HeadShotDamage;
					}
					else
					{
						if (ProjectileClass)
						{
							SpawnerProjectile = World->SpawnActor<AProjectileBase>(ServerSideRewindProjectileClass, SocketTransform.GetLocation(), TargetRotation, SpawnParams);
							if (SpawnerProjectile)
							{
								SpawnerProjectile->bUseServerSideRewind = true;
							}
						}
					}
				}
				else
				{
					if (InstigatorPawn->IsLocallyControlled())
					{
						if (ProjectileClass)
						{
							SpawnerProjectile = World->SpawnActor<AProjectileBase>(ProjectileClass, SocketTransform.GetLocation(), TargetRotation, SpawnParams);
						}
						SpawnerProjectile->bUseServerSideRewind = true;
						SpawnerProjectile->TraceStart = SocketTransform.GetLocation();
						SpawnerProjectile->InitialVelocity = SpawnerProjectile->GetActorForwardVector() * SpawnerProjectile->InitialSpeed;
					}
					else
					{
						if (ProjectileClass)
						{
							SpawnerProjectile = World->SpawnActor<AProjectileBase>(ServerSideRewindProjectileClass, SocketTransform.GetLocation(), TargetRotation, SpawnParams);
						}
						SpawnerProjectile->bUseServerSideRewind = false;
					}
				}
			}
			else
			{
				if (InstigatorPawn->HasAuthority())
				{
					if (ProjectileClass)
					{
						SpawnerProjectile = World->SpawnActor<AProjectileBase>(ProjectileClass, SocketTransform.GetLocation(), TargetRotation, SpawnParams);
					}
					SpawnerProjectile->bUseServerSideRewind = false;
					SpawnerProjectile->Damage = Damage;
					SpawnerProjectile->HeadShotDamage = HeadShotDamage;
				}
			}
		}
	}
}
