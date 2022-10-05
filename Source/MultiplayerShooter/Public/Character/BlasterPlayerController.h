// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "GameModes/BlasterGameMode.h"
#include "GameStates/BlasterGameState.h"
#include "HUD/CharacterOverlayWidget.h"
#include "Weapons/WeaponTypes.h"
#include "BlasterPlayerController.generated.h"

class UReturnToMainMenuWidget;
class ABlasterHUD;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FHighPingDelegate, bool, bPingTooHigh);

UCLASS()
class MULTIPLAYERSHOOTER_API ABlasterPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	float SingleTripTime = 0.f;
	FHighPingDelegate HighPingDelegate;
	
private:
	UPROPERTY(Transient)
	ABlasterHUD* BlasterHUD;

	UPROPERTY(ReplicatedUsing = OnRep_MatchState)
	FName MatchState;

	UPROPERTY(Transient)
	UCharacterOverlayWidget* CharacterOverlayWidget;

	UPROPERTY(Transient)
	ABlasterGameMode* BlasterGameMode;

	UPROPERTY(EditAnywhere)
	TSubclassOf<UUserWidget> ReturnToMainMenuWidgetClass;

	UPROPERTY()
	UReturnToMainMenuWidget* ReturnToMainMenuWidget;

	bool bReturnToMainMenuOpen = false;
	
	float MatchTime = 0.f;
	float CountdownInt = 0.f;
	float WarmupTime = 0.f;
	float LevelStartingTime = 0.f;
	float CooldownTime = 0.f;

	bool bInitializeHealth = false;
	bool bInitializeScore = false;
	bool bInitializeDefeats = false;
	bool bInitializeGrenades = false;
	bool bInitializeShield = false;
	bool bInitializeCarriedAmmo = false;
	bool bInitializeWeaponAmmo = false;

	float HUDHealth;
	float HUDMaxHealth;
	float HUDShield;
	float HUDMaxShield;
	float HUDScore;
	float HUDCarriedAmmo;
	float HUDWeaponAmmo;
	int32 HUDDefeats;
	int32 HUDGrenades;

	UPROPERTY(EditAnywhere)
	float HighPingDuration = 5.f;
	
	UPROPERTY(EditAnywhere)
	float CheckPingFrequency = 20.f;
	
	UPROPERTY(EditAnywhere)
	float HighPingThreshold = 50.f;
	
	float HighPingRunningTime = 0.f;
	float PingAnimationRunningTime = 0.f;
	
protected:
	UPROPERTY(EditAnywhere, Category = "Time")
	float TimeSyncFrequency = 5.f;
	float ClientServerDelta = 0.f;
	float TimeSyncRunningTime = 0.f;

	UPROPERTY(ReplicatedUsing = OnRep_ShowTeamScores)
	bool bShowTeamScores = false;

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

	UFUNCTION(Server, Reliable)
	void ServerReportPingStatus(bool bHighPing);

	UFUNCTION()
	void OnRep_ShowTeamScores();

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;
	virtual void SetupInputComponent() override;
	
	void SetHUDTime();
	void CheckTimeSync(float DeltaTime);

	void PollInit();

	void HighPingWarning();
	void StopHighPingWarning();
	void CheckPing(float DeltaTime);

	void ShowReturnToMainMenu();

	void ClientElimAnnouncement(const APlayerState* Attacker, const APlayerState* Victim);

	FString GetInfoText(const TArray<ABlasterPlayerState*>& Players);
	FString GetTeamsInfoText(ABlasterGameState* BlasterGameState);
	
public:
	virtual void OnPossess(APawn* InPawn) override;
	virtual void ReceivedPlayer() override;
	virtual float GetServerTime();

	void HandleCooldown();
	void HandleMatchHasStarted(bool bTeamsMatch = false);
	
	void SetHUDHealth(const float Health, const float MaxHealth);
	void SetHUDShield(const float Shield, const float MaxShield);
	void SetHUDScore(const float Score);
	void SetHUDDefeats(const int32 Defeats);
	void SetHUDWeaponAmmo(const int32 Ammo);
	void SetHUDCarriedAmmo(const int32 Ammo);
	void SetHUDWeaponType(const EWeaponType& WeaponType);
	void SetHUDMatchCountdown(const float CountdownTime);
	void SetHUDAnnouncementCountdown(const float CountdownTime);
	void SetHUDGrenades(const int32 Grenades);
	void SetHUDRedTeamScore(const int32 RedScore);
	void SetHUDBlueTeamScore(const int32 BlueScore);
	
	void BroadcastElim(const APlayerState* Attacker, const APlayerState* Victim);
	
	void OnMatchStateSet(FName State, bool bTeamMatch = false);

	void HideTeamScores();
	void InitTeamScores();
	
};
