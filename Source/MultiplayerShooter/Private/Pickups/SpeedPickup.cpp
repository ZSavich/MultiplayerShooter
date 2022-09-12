// Fill out your copyright notice in the Description page of Project Settings.

#include "Pickups/SpeedPickup.h"
#include "Character/BlasterCharacter.h"
#include "Components/BuffComponent.h"

void ASpeedPickup::OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
                                   UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	Super::OnSphereOverlap(OverlappedComponent, OtherActor, OtherComp, OtherBodyIndex, bFromSweep, SweepResult);
	if (const ABlasterCharacter* Character = Cast<ABlasterCharacter>(OtherActor))
	{
		if (UBuffComponent* BuffComponent = Character->GetBuffComponent())
		{
			BuffComponent->BuffSpeed(BaseSpeedBuff, CrouchSpeedBuff, SpeedBuffTime);
		}
	}
	Destroy();
}
