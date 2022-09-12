// Fill out your copyright notice in the Description page of Project Settings.

#include "Character/BlasterPlayerController.h"

#include "Character/BlasterCharacter.h"
#include "GameFramework/GameMode.h"
#include "GameModes/BlasterGameMode.h"
#include "GameStates/BlasterGameState.h"
#include "HUD/BlasterHUD.h"
#include "HUD/CharacterOverlayWidget.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"

void ABlasterPlayerController::GetLifetimeReplicatedProps(TArray< FLifetimeProperty >& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ABlasterPlayerController, MatchState);
}

void ABlasterPlayerController::ClientJoinMidgame_Implementation(FName StateOfMatch, float Warmup, float Match, float Cooldown,
	float StartingTime)
{
	WarmupTime = Warmup;
	MatchTime = Match;
	LevelStartingTime = StartingTime;
	MatchState = StateOfMatch;
	CooldownTime = Cooldown;
	OnMatchStateSet(MatchState);
	BlasterHUD = BlasterHUD == nullptr ? GetHUD<ABlasterHUD>() : BlasterHUD;
	if (BlasterHUD && MatchState == MatchState::WaitingToStart)
	{
		BlasterHUD->AddAnnouncementWidget();
	}
}

void ABlasterPlayerController::BeginPlay()
{
	Super::BeginPlay();
	ServerCheckMatchState();
	BlasterHUD = Cast<ABlasterHUD>(GetHUD());
}

void ABlasterPlayerController::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	SetHUDTime();
	CheckTimeSync(DeltaSeconds);
	PollInit();
}

void ABlasterPlayerController::CheckTimeSync(float DeltaTime)
{
	TimeSyncRunningTime += DeltaTime;
	if (IsLocalController() && TimeSyncRunningTime > TimeSyncFrequency)
	{
		ServerRequestServerTime(GetWorld()->GetTimeSeconds());
		TimeSyncRunningTime = 0.f;
	}
}

void ABlasterPlayerController::PollInit()
{
	if (CharacterOverlayWidget == nullptr)
	{
		if (BlasterHUD && BlasterHUD->CharacterOverlay)
		{
			CharacterOverlayWidget = BlasterHUD->CharacterOverlay;
			if (CharacterOverlayWidget)
			{
				if (bInitializeHealth) SetHUDHealth(HUDHealth, HUDMaxHealth);
				if (bInitializeShield) SetHUDShield(HUDShield, HUDMaxShield);
				if (bInitializeScore) SetHUDScore(HUDScore);
				if (bInitializeDefeats) SetHUDDefeats(HUDDefeats);
				if (bInitializeCarriedAmmo) SetHUDCarriedAmmo(HUDCarriedAmmo);
				if (bInitializeWeaponAmmo) SetHUDWeaponAmmo(HUDWeaponAmmo);

				if (const ABlasterCharacter* BlasterCharacter = GetPawn<ABlasterCharacter>())
				{
					if (BlasterCharacter->GetCombatComponent())
					{
						HUDGrenades = BlasterCharacter->GetCombatComponent()->GetGrenades();
					}
				}
				if (bInitializeGrenades) SetHUDGrenades(HUDGrenades);
			}
		}
	}
}

void ABlasterPlayerController::HandleMatchHasStarted()
{
	BlasterHUD = BlasterHUD == nullptr ? GetHUD<ABlasterHUD>() : BlasterHUD;
	if (BlasterHUD)
	{
		if (BlasterHUD->CharacterOverlay == nullptr)
		{
			BlasterHUD->AddCharacterOverlay();
		}
		if (BlasterHUD->AnnouncementWidget)
		{
			BlasterHUD->AnnouncementWidget->SetVisibility(ESlateVisibility::Hidden);
		}
	}
}

void ABlasterPlayerController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);
	if (ABlasterCharacter* BlasterCharacter = Cast<ABlasterCharacter>(InPawn))
	{
		SetHUDHealth(BlasterCharacter->GetHealth(), BlasterCharacter->GetMaxHealth());
	}
}

void ABlasterPlayerController::ReceivedPlayer()
{
	Super::ReceivedPlayer();
	if (IsLocalController())
	{
		ServerRequestServerTime(GetWorld()->GetTimeSeconds());
	}
}

float ABlasterPlayerController::GetServerTime()
{
	if (HasAuthority())
	{
		return GetWorld()->GetTimeSeconds();
	}
	return GetWorld()->GetTimeSeconds() + ClientServerDelta;
}

void ABlasterPlayerController::HandleCooldown()
{
	BlasterHUD = BlasterHUD == nullptr ? GetHUD<ABlasterHUD>() : BlasterHUD;
	if (BlasterHUD)
	{
		BlasterHUD->CharacterOverlay->RemoveFromParent();
		if (BlasterHUD->AnnouncementWidget)
		{
			BlasterHUD->AnnouncementWidget->SetVisibility(ESlateVisibility::Visible);
			if (BlasterHUD->AnnouncementWidget->AnnouncementText)
			{
				const FString AnnouncementText = TEXT("New Match Starts In:");
				BlasterHUD->AnnouncementWidget->AnnouncementText->SetText(FText::FromString(AnnouncementText));
			}
			if (BlasterHUD->AnnouncementWidget->InfoText)
			{
				FString InfoText = TEXT("");
				if (ABlasterGameState* BlasterGameState = Cast<ABlasterGameState>(UGameplayStatics::GetGameState(this)))
				{
					if (ABlasterPlayerState* BlasterPlayerState = GetPlayerState<ABlasterPlayerState>())
					{
						TArray<ABlasterPlayerState*>& TopPlayers = BlasterGameState->TopScoringPlayers;
						if (TopPlayers.Num() == 0)
						{
							InfoText = TEXT("There is no winner.");
						}
						else if (TopPlayers.Num() == 1 && TopPlayers[0] == BlasterPlayerState)
						{
							InfoText = TEXT("You are the winner.");
						}
						else if (TopPlayers.Num() == 1)
						{
							InfoText = FString::Printf(TEXT("Winner: \n%s"), *TopPlayers[0]->GetPlayerName());
						}
						else if (TopPlayers.Num() > 1)
						{
							InfoText = TEXT("Players tied for the win:\n");
							for (const auto& TopPlayer : TopPlayers)
							{
								InfoText.Append(FString::Printf(TEXT("%s\n"), *TopPlayer->GetPlayerName()));
							}
						}
					}
				}
				
				BlasterHUD->AnnouncementWidget->InfoText->SetText(FText::FromString(InfoText));
			}
		}
	}
	if (ABlasterCharacter* BlasterCharacter = GetPawn<ABlasterCharacter>())
	{
		BlasterCharacter->bDisableGameplay = true;
		if (UCombatComponent* CombatComponent = BlasterCharacter->GetCombatComponent())
		{
			CombatComponent->FireButtonPressed(false);
		}
	}
}

void ABlasterPlayerController::SetHUDHealth(const float Health, const float MaxHealth)
{
	BlasterHUD = BlasterHUD == nullptr ? Cast<ABlasterHUD>(GetHUD()) : BlasterHUD;
	const bool bHUDValid =
		BlasterHUD &&
		BlasterHUD->CharacterOverlay &&
		BlasterHUD->CharacterOverlay->HealthBar &&
		BlasterHUD->CharacterOverlay->HealthText;

	if (bHUDValid)
	{
		const float HealthPercent = Health / MaxHealth;
		BlasterHUD->CharacterOverlay->HealthBar->SetPercent(HealthPercent);

		const FString HealthText = FString::Printf(TEXT("%d / %d"), FMath::CeilToInt(Health), FMath::CeilToInt(MaxHealth));
		BlasterHUD->CharacterOverlay->HealthText->SetText(FText::FromString(HealthText));
	}
	else
	{
		bInitializeHealth = true;
		HUDHealth = Health;
		HUDMaxHealth = MaxHealth;
	}
}

void ABlasterPlayerController::SetHUDShield(const float Shield, const float MaxShield)
{
	BlasterHUD = BlasterHUD == nullptr ? Cast<ABlasterHUD>(GetHUD()) : BlasterHUD;
	const bool bHUDValid =
		BlasterHUD &&
		BlasterHUD->CharacterOverlay &&
		BlasterHUD->CharacterOverlay->ShieldBar &&
		BlasterHUD->CharacterOverlay->ShieldText;

	if (bHUDValid)
	{
		const float HealthPercent = Shield / MaxShield;
		BlasterHUD->CharacterOverlay->ShieldBar->SetPercent(HealthPercent);

		const FString ShieldText = FString::Printf(TEXT("%d / %d"), FMath::CeilToInt(Shield), FMath::CeilToInt(MaxShield));
		BlasterHUD->CharacterOverlay->ShieldText->SetText(FText::FromString(ShieldText));
	}
	else
	{
		bInitializeShield = true;
		HUDShield = Shield;
		HUDMaxShield = MaxShield;
	}
}

void ABlasterPlayerController::SetHUDScore(const float Score)
{
	BlasterHUD = BlasterHUD == nullptr ? GetHUD<ABlasterHUD>() : BlasterHUD;
	const bool bHUDValid =
		BlasterHUD &&
		BlasterHUD->CharacterOverlay &&
		BlasterHUD->CharacterOverlay->ScoreAmount;
	if (bHUDValid)
	{
		const FString ScoreText = FString::Printf(TEXT("%d"), FMath::FloorToInt(Score));
		BlasterHUD->CharacterOverlay->ScoreAmount->SetText(FText::FromString(ScoreText));
	}
	else
	{
		bInitializeScore = true;
		HUDScore = Score;
	}
}

void ABlasterPlayerController::SetHUDDefeats(const int32 Defeats)
{
	BlasterHUD = BlasterHUD == nullptr ? GetHUD<ABlasterHUD>() : BlasterHUD;
	const bool bHUDValid =
		BlasterHUD &&
		BlasterHUD->CharacterOverlay &&
		BlasterHUD->CharacterOverlay->DefeatsAmount;
	if (bHUDValid)
	{
		const FString DefeatsText = FString::Printf(TEXT("%d"), Defeats);
		BlasterHUD->CharacterOverlay->DefeatsAmount->SetText(FText::FromString(DefeatsText));
	}
	else
	{
		bInitializeDefeats = true;
		HUDDefeats = Defeats;
	}
}

void ABlasterPlayerController::SetHUDWeaponAmmo(const int32 Ammo)
{
	BlasterHUD = BlasterHUD == nullptr ? GetHUD<ABlasterHUD>() : BlasterHUD;
	const bool bHUDValid =
		BlasterHUD &&
		BlasterHUD->CharacterOverlay &&
		BlasterHUD->CharacterOverlay->WeaponAmmoAmount;
	if (bHUDValid)
	{
		const FString WeaponAmmoText = FString::Printf(TEXT("%d"), Ammo);
		BlasterHUD->CharacterOverlay->WeaponAmmoAmount->SetText(FText::FromString(WeaponAmmoText));
	}
	else
	{
		bInitializeWeaponAmmo = true;
		HUDWeaponAmmo = Ammo;
	}
}

void ABlasterPlayerController::SetHUDCarriedAmmo(const int32 Ammo)
{
	BlasterHUD = BlasterHUD == nullptr ? GetHUD<ABlasterHUD>() : BlasterHUD;
	const bool bHUDValid =
		BlasterHUD &&
		BlasterHUD->CharacterOverlay &&
		BlasterHUD->CharacterOverlay->CarriedAmmoAmount;
	if (bHUDValid)
	{
		const FString WeaponAmmoText = FString::Printf(TEXT("%d"), Ammo);
		BlasterHUD->CharacterOverlay->CarriedAmmoAmount->SetText(FText::FromString(WeaponAmmoText));
	}
	else
	{
		bInitializeCarriedAmmo = true;
		HUDCarriedAmmo = Ammo;
	}
}

void ABlasterPlayerController::SetHUDWeaponType(const EWeaponType& WeaponType)
{
	BlasterHUD = BlasterHUD == nullptr ? GetHUD<ABlasterHUD>() : BlasterHUD;
	const bool bHUDValid =
		BlasterHUD &&
		BlasterHUD->CharacterOverlay &&
		BlasterHUD->CharacterOverlay->WeaponTypeText;
	if (bHUDValid)
	{
		FString WeaponTypeText;
		switch (WeaponType)
		{
		case EWeaponType::EWT_AssaultRifle:
			WeaponTypeText = TEXT("Rifle Weapon");
			break;
		case EWeaponType::EWT_RocketLauncher:
			WeaponTypeText = TEXT("Rocket Launcher");
			break;
		case EWeaponType::EWT_Pistol:
			WeaponTypeText = TEXT("Pistol");
			break;
		case EWeaponType::EWT_SubmachineGun:
			WeaponTypeText = TEXT("SMG");
			break;
		case EWeaponType::EWT_Shotgun:
			WeaponTypeText = TEXT("Shotgun");
			break;
		case EWeaponType::EWT_SniperRifle:
			WeaponTypeText = TEXT("Sniper Rifle");
			break;
		case EWeaponType::EWT_GrenadeLauncher:
			WeaponTypeText = TEXT("Grenade Launcher");
			break;
		case EWeaponType::EWT_MAX:
			WeaponTypeText = TEXT("Unknown Weapon");
			break;
		default:
			WeaponTypeText = TEXT("");
			break;
		}
		BlasterHUD->CharacterOverlay->WeaponTypeText->SetText(FText::FromString(WeaponTypeText));
	}
}

void ABlasterPlayerController::SetHUDMatchCountdown(const float CountdownTime)
{
	BlasterHUD = BlasterHUD == nullptr ? GetHUD<ABlasterHUD>() : BlasterHUD;
	const bool bHUDValid =
		BlasterHUD &&
		BlasterHUD->CharacterOverlay &&
		BlasterHUD->CharacterOverlay->MatchCountdownText;
	if (bHUDValid)
	{
		if (CountdownTime < 0.f)
		{
			BlasterHUD->CharacterOverlay->MatchCountdownText->SetText(FText());
			return;
		}
		const int32 Minutes = FMath::FloorToInt(CountdownTime / 60);
		const int32 Seconds = CountdownTime - Minutes * 60;
		const FString CountdownText = FString::Printf(TEXT("%02d:%02d"), Minutes, Seconds);
		BlasterHUD->CharacterOverlay->MatchCountdownText->SetText(FText::FromString(CountdownText));
	}
}

void ABlasterPlayerController::SetHUDAnnouncementCountdown(const float CountdownTime)
{
	BlasterHUD = BlasterHUD == nullptr ? GetHUD<ABlasterHUD>() : BlasterHUD;
	const bool bHUDValid  =
		BlasterHUD &&
			BlasterHUD->AnnouncementWidget &&
				BlasterHUD->AnnouncementWidget->WarmupTime;
	if (bHUDValid)
	{
		if (CountdownTime < 0.f)
		{
			BlasterHUD->AnnouncementWidget->WarmupTime->SetText(FText());
			return;
		}
		const int32 Minutes = FMath::FloorToInt(CountdownTime / 60);
		const int32 Seconds = CountdownTime - Minutes * 60;
		const FString CountdownText = FString::Printf(TEXT("%02d:%02d"), Minutes, Seconds);
		BlasterHUD->AnnouncementWidget->WarmupTime->SetText(FText::FromString(CountdownText));
	}
}

void ABlasterPlayerController::SetHUDGrenades(const int32 Grenades)
{
	BlasterHUD = BlasterHUD == nullptr ? GetHUD<ABlasterHUD>() : BlasterHUD;
	const bool bHUDValid =
		BlasterHUD &&
		BlasterHUD->CharacterOverlay &&
		BlasterHUD->CharacterOverlay->GrenadesText;
	if (bHUDValid)
	{
		const FString GrenadesString = FString::Printf(TEXT("%d"), Grenades);
		BlasterHUD->CharacterOverlay->GrenadesText->SetText(FText::FromString(GrenadesString));
	}
	else
	{
		bInitializeGrenades = true;
		HUDGrenades = Grenades;
	}
}

void ABlasterPlayerController::OnMatchStateSet(FName State)
{
	MatchState = State;
	if (MatchState == MatchState::InProgress)
	{
		HandleMatchHasStarted();
	}
	else if (MatchState == MatchState::Cooldown)
	{
		HandleCooldown();
	}
}

void ABlasterPlayerController::SetHUDTime()
{
	float TimeLeft = 0.f;
	if (MatchState == MatchState::WaitingToStart)
	{
		TimeLeft = WarmupTime - GetServerTime() + LevelStartingTime;
	}
	else if (MatchState == MatchState::InProgress)
	{
		TimeLeft = WarmupTime + MatchTime - GetServerTime() + LevelStartingTime;
	}
	else if (MatchState == MatchState::Cooldown)
	{
		TimeLeft = CooldownTime + WarmupTime + MatchTime - GetServerTime() + LevelStartingTime;
	}
	
	int32 SecondsLeft = FMath::CeilToInt(MatchTime - TimeLeft);
	
	if (CountdownInt != SecondsLeft)
	{
		if (MatchState == MatchState::WaitingToStart || MatchState == MatchState::Cooldown)
		{
			SetHUDAnnouncementCountdown(TimeLeft);
		}
		else if (MatchState == MatchState::InProgress)
		{
			SetHUDMatchCountdown(TimeLeft);
		}
	}
	CountdownInt = SecondsLeft;
}

void ABlasterPlayerController::ClientReportServerTime_Implementation(float TimeOfClientRequest,
	float TimeServerReceivedClientRequest)
{
	const float RoundTripTime = GetWorld()->GetTimeSeconds() - TimeOfClientRequest;
	const float CurrentServerTime = TimeServerReceivedClientRequest + (0.5f * RoundTripTime);
	ClientServerDelta = CurrentServerTime - GetWorld()->GetTimeSeconds();
}

void ABlasterPlayerController::ServerRequestServerTime_Implementation(float TimeOfClientRequest)
{
	const float ServerTimeOfReceipt = GetWorld()->GetTimeSeconds();
	ClientReportServerTime(TimeOfClientRequest, ServerTimeOfReceipt);
}

void ABlasterPlayerController::OnRep_MatchState()
{
	if (MatchState == MatchState::InProgress)
	{
		HandleMatchHasStarted();
	}
	else if (MatchState == MatchState::Cooldown)
	{
		HandleCooldown();
	}
}

void ABlasterPlayerController::ServerCheckMatchState_Implementation()
{
	BlasterGameMode = BlasterGameMode == nullptr ? Cast<ABlasterGameMode>(UGameplayStatics::GetGameMode(this)) : BlasterGameMode;
	if (BlasterGameMode)
	{
		WarmupTime = BlasterGameMode->WarmupTime;
		MatchTime = BlasterGameMode->MatchTime;
		LevelStartingTime = BlasterGameMode->LevelStartingTime;
		MatchState = BlasterGameMode->GetMatchState();
		CooldownTime = BlasterGameMode->CooldownTime;
		ClientJoinMidgame(MatchState, WarmupTime, MatchTime, CooldownTime, LevelStartingTime);
	}
}
