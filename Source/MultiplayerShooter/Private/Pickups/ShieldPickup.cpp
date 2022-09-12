// Fill out your copyright notice in the Description page of Project Settings.

#include "Pickups/ShieldPickup.h"
#include "Character/BlasterCharacter.h"
#include "Components/BuffComponent.h"

void AShieldPickup::OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
                                    UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	Super::OnSphereOverlap(OverlappedComponent, OtherActor, OtherComp, OtherBodyIndex, bFromSweep, SweepResult);
	if (const ABlasterCharacter* Character = Cast<ABlasterCharacter>(OtherActor))
	{
		if (UBuffComponent* BuffComponent = Character->GetBuffComponent())
		{
			BuffComponent->ReplenishShield(ShieldReplenishAmount, ShieldReplenishTime);
		}
	}
	Destroy();
}
