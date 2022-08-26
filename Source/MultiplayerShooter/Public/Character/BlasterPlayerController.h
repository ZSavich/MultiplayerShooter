// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "BlasterPlayerController.generated.h"

class ABlasterHUD;

UCLASS()
class MULTIPLAYERSHOOTER_API ABlasterPlayerController : public APlayerController
{
	GENERATED_BODY()

private:
	UPROPERTY()
	ABlasterHUD* BlasterHUD;

protected:
	virtual void BeginPlay() override;

public:
	virtual void OnPossess(APawn* InPawn) override;
	
	void SetHUDHealth(const float Health, const float MaxHealth);
	void SetHUDScore(const float Score);
	void SetHUDDefeats(const int32 Defeats);
	void SetHUDWeaponAmmo(const int32 Ammo);
};
