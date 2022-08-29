// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "GameModes/BlasterGameMode.h"
#include "HUD/CharacterOverlayWidget.h"
#include "Weapons/WeaponTypes.h"
#include "BlasterPlayerController.generated.h"

class ABlasterHUD;

UCLASS()
class MULTIPLAYERSHOOTER_API ABlasterPlayerController : public APlayerController
{
	GENERATED_BODY()

private:
	UPROPERTY(Transient)
	ABlasterHUD* BlasterHUD;

	UPROPERTY(ReplicatedUsing = OnRep_MatchState)
	FName MatchState;

	UPROPERTY(Transient)
	UCharacterOverlayWidget* CharacterOverlayWidget;

	UPROPERTY(Transient)
	ABlasterGameMode* BlasterGameMode;
	
	float MatchTime = 0.f;
	float CountdownInt = 0.f;
	float WarmupTime = 0.f;
	float LevelStartingTime = 0.f;
	float CooldownTime = 0.f;

	bool bInitializedCharacterOverlay = false;

	float HUDHealth;
	float HUDMaxHealth;
	float HUDScore;
	int32 HUDDefeats;

protected:
	UPROPERTY(EditAnywhere, Category = "Time")
	float TimeSyncFrequency = 5.f;
	float ClientServerDelta = 0.f;
	float TimeSyncRunningTime = 0.f;

private:
	UFUNCTION()
	void OnRep_MatchState();
	
protected:
	UFUNCTION(Server, Reliable)
	void ServerRequestServerTime(float TimeOfClientRequest);

	UFUNCTION(Server, Reliable)
	void ServerCheckMatchState();

	UFUNCTION(Client, Reliable)
	void ClientReportServerTime(float TimeOfClientRequest, float TimeServerReceivedClientRequest);

	UFUNCTION(Client, Reliable)
	void ClientJoinMidgame(FName StateOfMatch, float Warmup, float Match, float Cooldown, float StartingTime);

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;
	
	void SetHUDTime();
	void CheckTimeSync(float DeltaTime);

	void PollInit();

public:
	virtual void OnPossess(APawn* InPawn) override;
	virtual void ReceivedPlayer() override;
	virtual float GetServerTime();

	void HandleCooldown();
	void HandleMatchHasStarted();
	
	void SetHUDHealth(const float Health, const float MaxHealth);
	void SetHUDScore(const float Score);
	void SetHUDDefeats(const int32 Defeats);
	void SetHUDWeaponAmmo(const int32 Ammo);
	void SetHUDCarriedAmmo(const int32 Ammo);
	void SetHUDWeaponType(const EWeaponType& WeaponType);
	void SetHUDMatchCountdown(const float CountdownTime);
	void SetHUDAnnouncementCountdown(const float CountdownTime);
	
	void OnMatchStateSet(FName State);
};
