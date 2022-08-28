// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "Weapons/WeaponTypes.h"
#include "BlasterPlayerController.generated.h"

class ABlasterHUD;

UCLASS()
class MULTIPLAYERSHOOTER_API ABlasterPlayerController : public APlayerController
{
	GENERATED_BODY()

private:
	UPROPERTY()
	ABlasterHUD* BlasterHUD;
	float MatchTime = 120.f;
	float CountdownInt = 0.f;

protected:
	UPROPERTY(EditAnywhere, Category = "Time")
	float TimeSyncFrequency = 5.f;
	float ClientServerDelta = 0.f;
	float TimeSyncRunningTime = 0.f;
	
	UFUNCTION(Server, Reliable)
	void ServerRequestServerTime(float TimeOfClientRequest);

	UFUNCTION(Client, Reliable)
	void ClientReportServerTime(float TimeOfClientRequest, float TimeServerReceivedClientRequest);

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;
	
	void SetHUDTime();
	void CheckTimeSync(float DeltaTime);

public:
	virtual void OnPossess(APawn* InPawn) override;
	virtual void ReceivedPlayer() override;
	virtual float GetServerTime();
	
	
	void SetHUDHealth(const float Health, const float MaxHealth);
	void SetHUDScore(const float Score);
	void SetHUDDefeats(const int32 Defeats);
	void SetHUDWeaponAmmo(const int32 Ammo);
	void SetHUDCarriedAmmo(const int32 Ammo);
	void SetHUDWeaponType(const EWeaponType& WeaponType);
	void SetHUDMatchCountdown(const float CountdownTime);
};
