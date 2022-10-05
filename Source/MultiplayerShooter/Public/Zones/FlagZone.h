// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BlasterTypes/Team.h"
#include "Components/SphereComponent.h"
#include "GameFramework/Actor.h"
#include "FlagZone.generated.h"

UCLASS()
class MULTIPLAYERSHOOTER_API AFlagZone : public AActor
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere)
	ETeam Team;

private:
	UPROPERTY(EditAnywhere)
	USphereComponent* ZoneSphere;
	
public:	
	AFlagZone();

protected:
	virtual void BeginPlay() override;

	UFUNCTION()
	virtual void OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
};
