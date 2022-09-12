// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Pickups/PickupBase.h"
#include "JumpPickup.generated.h"

UCLASS()
class MULTIPLAYERSHOOTER_API AJumpPickup : public APickupBase
{
	GENERATED_BODY()

private:
	UPROPERTY(EditAnywhere)
	float JumpZVelocityBuff = 2000.f;

	UPROPERTY(EditAnywhere)
	float JumpBuffTime = 15.f;
	
protected:
	virtual void OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult) override;
	
};
