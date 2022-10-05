// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Weapons/WeaponBase.h"
#include "Flag.generated.h"

/**
 * 
 */
UCLASS()
class MULTIPLAYERSHOOTER_API AFlag : public AWeaponBase
{
	GENERATED_BODY()

private:
	UPROPERTY(VisibleAnywhere)
	UStaticMeshComponent* FlagMesh;
	
	FTransform InitialTransform;

public:
	AFlag();

	void ResetFlag();
	
	virtual void Dropped() override;
	virtual void OnDropped() override;
	virtual void OnEquipped() override;
	virtual void BeginPlay() override;

	FORCEINLINE FTransform GetInitialTransform() { return InitialTransform; }
};
