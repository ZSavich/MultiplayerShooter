// Fill out your copyright notice in the Description page of Project Settings.

#include "Weapons/HitScanWeapon.h"
#include "Character/BlasterCharacter.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "Particles/ParticleSystemComponent.h"
#include "Sound/SoundCue.h"
#include "Weapons/WeaponTypes.h"

void AHitScanWeapon::Fire(const FVector& HitTarget)
{
	Super::Fire(HitTarget);

	APawn* OwnerPawn = GetOwner<APawn>();
	if (OwnerPawn == nullptr)
	{
		return;
	}
	AController* InstigatorController = OwnerPawn->GetController();

	const USkeletalMeshSocket* MuzzleFlashSocket = GetWeaponMesh()->GetSocketByName(TEXT("MuzzleFlash"));
	if (MuzzleFlashSocket)
	{
		const FTransform SocketTransform = MuzzleFlashSocket->GetSocketTransform(GetWeaponMesh());
		const FVector Start = SocketTransform.GetLocation();

		FHitResult FireHit;
		WeaponTraceHit(Start, HitTarget, FireHit);

		ABlasterCharacter* BlasterCharacter = Cast<ABlasterCharacter>(FireHit.GetActor());
		if (BlasterCharacter && InstigatorController && HasAuthority())
		{
			UGameplayStatics::ApplyDamage(
				BlasterCharacter,
				Damage,
				InstigatorController,
				this,
				UDamageType::StaticClass()
			);
		}
		if (ImpactParticleFX)
		{
			UGameplayStatics::SpawnEmitterAtLocation(
				GetWorld(),
				ImpactParticleFX,
				FireHit.ImpactPoint,
				FireHit.ImpactPoint.Rotation()
			);
		}
		if (HitSoundFX)
		{
			UGameplayStatics::PlaySoundAtLocation(
				this,
				HitSoundFX,
				FireHit.ImpactPoint
			);
		}
		if (MuzzleFlashParticleFX)
		{
			UGameplayStatics::SpawnEmitterAtLocation(
				GetWorld(),
				MuzzleFlashParticleFX,
				SocketTransform
			);
		}
		if (FireSoundFX)
		{
			UGameplayStatics::PlaySoundAtLocation(
				this,
				FireSoundFX,
				GetActorLocation()
			);
		}
	}
}

FVector AHitScanWeapon::TraceEndWithScatter(const FVector& TraceStart, const FVector& HitTarget)
{
	const FVector ToTargetNormalized = (HitTarget - TraceStart).GetSafeNormal();
	const FVector SphereCenter = TraceStart + ToTargetNormalized * DistanceToSphere;

	const FVector RandVector = UKismetMathLibrary::RandomUnitVector() * FMath::FRandRange(0.f, SphereRadius);
	const FVector EndLoc = SphereCenter + RandVector;
	const FVector ToEndLoc = EndLoc - TraceStart;
	const FVector EndResult = FVector(TraceStart + ToEndLoc * TRACE_LENGTH / ToEndLoc.Size());

	/* 
	* Debug Scatter 
	DrawDebugSphere(GetWorld(), SphereCenter, SphereRadius, 12.f, FColor::Red, true);
	DrawDebugSphere(GetWorld(), EndLoc, 4.f, 12.f, FColor::Orange, true);
	DrawDebugLine(GetWorld(), TraceStart, EndResult, FColor::Cyan, true);
	*/
	
	return EndResult;
}

void AHitScanWeapon::WeaponTraceHit(const FVector& TraceStart, const FVector& HitTarget, FHitResult& OutHit)
{
	if (UWorld* World = GetWorld())
	{
		const FVector End = bUseScatter ? TraceEndWithScatter(TraceStart, HitTarget) : TraceStart + (HitTarget - TraceStart) * 1.25f;
		World->LineTraceSingleByChannel(
			OutHit,
			TraceStart,
			End,
			ECC_Visibility
		);
		FVector BeamEnd = End;
		if (OutHit.bBlockingHit)
		{
			BeamEnd = OutHit.ImpactPoint;
		}
		if (BeamParticleFX)
		{
			UParticleSystemComponent* BeamParticleComponent = UGameplayStatics::SpawnEmitterAtLocation(
				World,
				BeamParticleFX,
				TraceStart,
				FRotator::ZeroRotator
			);
			if (BeamParticleComponent)
			{
				BeamParticleComponent->SetVectorParameter(TEXT("Target"), BeamEnd);
			}
		}
	}
}
