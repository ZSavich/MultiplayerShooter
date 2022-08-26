// Fill out your copyright notice in the Description page of Project Settings.

#include "Weapons/ProjectileWeapon.h"

#include "Engine/SkeletalMeshSocket.h"
#include "Projectiles/ProjectileBase.h"

void AProjectileWeapon::Fire(const FVector& HitTarget)
{
	Super::Fire(HitTarget);

	if (!HasAuthority())
	{
		return;
	}
	
	APawn* InstigatorPawn = Cast<APawn>(GetOwner());
	const USkeletalMeshSocket* MuzzleFlashSocket = GetWeaponMesh()->GetSocketByName(TEXT("MuzzleFlash"));
	if (InstigatorPawn && MuzzleFlashSocket)
	{
		const FTransform SocketTransform = MuzzleFlashSocket->GetSocketTransform(GetWeaponMesh());
		const FVector ToTarget = HitTarget - SocketTransform.GetLocation();
		const FRotator TargetRotation = ToTarget.Rotation();

		FActorSpawnParameters SpawnParams;
		SpawnParams.Owner = GetOwner();
		SpawnParams.Instigator = InstigatorPawn;
		
		if (ProjectileClass)
		{
			if (UWorld* World = GetWorld())
			{
				World->SpawnActor<AProjectileBase>(ProjectileClass, SocketTransform.GetLocation(), TargetRotation, SpawnParams);
			}
		}
	}
}
