// Fill out your copyright notice in the Description page of Project Settings.

#include "Character/BlasterPlayerController.h"

#include "BlasterTypes/Announcement.h"
#include "Character/BlasterCharacter.h"
#include "GameFramework/GameMode.h"
#include "GameModes/BlasterGameMode.h"
#include "GameStates/BlasterGameState.h"
#include "HUD/AnnouncementWidget.h"
#include "HUD/BlasterHUD.h"
#include "HUD/CharacterOverlayWidget.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"
#include "HUD/ReturnToMainMenuWidget.h"

void ABlasterPlayerController::GetLifetimeReplicatedProps(TArray< FLifetimeProperty >& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ABlasterPlayerController, MatchState);
	DOREPLIFETIME(ABlasterPlayerController, bShowTeamScores);
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

void ABlasterPlayerController::ServerReportPingStatus_Implementation(bool bHighPing)
{
	HighPingDelegate.Broadcast(bHighPing);
}

void ABlasterPlayerController::OnRep_ShowTeamScores()
{
	if (bShowTeamScores)
	{
		InitTeamScores();
	}
	else
	{
		HideTeamScores();
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
	CheckPing(DeltaSeconds);
}

void ABlasterPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	if (InputComponent == nullptr)
	{
		return;
	}
	InputComponent->BindAction("Quit", IE_Pressed, this, &ABlasterPlayerController::ShowReturnToMainMenu);
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

void ABlasterPlayerController::HandleMatchHasStarted(bool bTeamsMatch)
{
	if (HasAuthority())
	{
		bShowTeamScores = bTeamsMatch;
	}
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
		if (!HasAuthority())
		{
			return;
		}
		if (bTeamsMatch)
		{
			InitTeamScores();
		}
		else
		{
			HideTeamScores();
		}
	}
}

FString ABlasterPlayerController::GetInfoText(const TArray<ABlasterPlayerState*>& Players)
{
	ABlasterPlayerState* BlasterPlayerState = GetPlayerState<ABlasterPlayerState>();
	if (BlasterPlayerState == nullptr)
	{
		return FString();
	}
	FString InfoText;
	if (ABlasterGameState* BlasterGameState = Cast<ABlasterGameState>(UGameplayStatics::GetGameState(this)))
	{
		if (Players.Num() == 0)
		{
			InfoText = Announcement::ThereIsNoWinner;
		}
		else if (Players.Num() == 1 && Players[0] == BlasterPlayerState)
		{
			InfoText = Announcement::YouAreTheWinner;
		}
		else if (Players.Num() == 1)
		{
			InfoText = FString::Printf(TEXT("Winner: \n%s"), *Players[0]->GetPlayerName());
		}
		else if (Players.Num() > 1)
		{
			InfoText = Announcement::PlayersTiedForTheWin;
			InfoText.Append(TEXT("\n"));
			for (const auto& TopPlayer : Players)
			{
				InfoText.Append(FString::Printf(TEXT("%s\n"), *TopPlayer->GetPlayerName()));
			}
		}
	}
	return InfoText;
}

FString ABlasterPlayerController::GetTeamsInfoText(ABlasterGameState* BlasterGameState)
{
	if (BlasterGameState == nullptr)
	{
		return FString();
	}

	FString InfoText;

	const int32 RedTeamScore = BlasterGameState->RedTeamScore;
	const int32 BlueTeamScore = BlasterGameState->BlueTeamScore;
	if (RedTeamScore == 0 && BlueTeamScore == 0)
	{
		InfoText = Announcement::ThereIsNoWinner;
	}
	else if (RedTeamScore == BlueTeamScore)
	{
		InfoText = FString::Printf(TEXT("%s\n"), *Announcement::TeamsTiedForTheWin);
		InfoText.Append(FString::Printf(TEXT("%s\n%s\n"), *Announcement::RedTeam, *Announcement::BlueTeam));
	}
	else if (RedTeamScore > BlueTeamScore)
	{
		InfoText = Announcement::RedTeamWins;
		InfoText.Append(FString::Printf(TEXT("\n%s: %d\n%s: %d\n"), *Announcement::RedTeam, RedTeamScore, *Announcement::BlueTeam, BlueTeamScore));
	}
	else if (BlueTeamScore > RedTeamScore)
	{
		InfoText = Announcement::BlueTeamWins;
		InfoText.Append(FString::Printf(TEXT("\n%s: %d\n%s: %d"), *Announcement::BlueTeam, BlueTeamScore, *Announcement::RedTeam, RedTeamScore));
	}
	return InfoText;
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
				const FString AnnouncementText = Announcement::NewMatchStartsIn;
				BlasterHUD->AnnouncementWidget->AnnouncementText->SetText(FText::FromString(AnnouncementText));
			}
			ABlasterGameState* BlasterGameState = Cast<ABlasterGameState>(UGameplayStatics::GetGameState(this));
			if (BlasterHUD->AnnouncementWidget->InfoText && BlasterGameState)
			{
				const TArray<ABlasterPlayerState*> TopPlayers = BlasterGameState->TopScoringPlayers;
				const FString InfoText = bShowTeamScores ? GetTeamsInfoText(BlasterGameState) : GetInfoText(TopPlayers);
				
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

void ABlasterPlayerController::SetHUDRedTeamScore(const int32 RedScore)
{
	BlasterHUD = BlasterHUD == nullptr ? Cast<ABlasterHUD>(GetHUD()) : BlasterHUD;
	const bool bHUDValid =
		BlasterHUD &&
		BlasterHUD->CharacterOverlay &&
		BlasterHUD->CharacterOverlay->RedTeamScoreText;

	if (bHUDValid)
	{
		BlasterHUD->CharacterOverlay->RedTeamScoreText->SetText(FText::AsNumber(RedScore));
	}
}

void ABlasterPlayerController::SetHUDBlueTeamScore(const int32 BlueScore)
{
	BlasterHUD = BlasterHUD == nullptr ? Cast<ABlasterHUD>(GetHUD()) : BlasterHUD;
	const bool bHUDValid =
		BlasterHUD &&
		BlasterHUD->CharacterOverlay &&
		BlasterHUD->CharacterOverlay->BlueTeamScoreText;

	if (bHUDValid)
	{
		BlasterHUD->CharacterOverlay->BlueTeamScoreText->SetText(FText::AsNumber(BlueScore));
	}
}

void ABlasterPlayerController::BroadcastElim(const APlayerState* Attacker, const APlayerState* Victim)
{
	ClientElimAnnouncement(Attacker, Victim);
}

void ABlasterPlayerController::OnMatchStateSet(FName State, bool bTeamMatch)
{
	MatchState = State;
	if (MatchState == MatchState::InProgress)
	{
		HandleMatchHasStarted(bTeamMatch);
	}
	else if (MatchState == MatchState::Cooldown)
	{
		HandleCooldown();
	}
}

void ABlasterPlayerController::HideTeamScores()
{
	BlasterHUD = BlasterHUD == nullptr ? Cast<ABlasterHUD>(GetHUD()) : BlasterHUD;
	const bool bHUDValid =
		BlasterHUD &&
		BlasterHUD->CharacterOverlay &&
		BlasterHUD->CharacterOverlay->RedTeamScoreText &&
		BlasterHUD->CharacterOverlay->BlueTeamScoreText &&
		BlasterHUD->CharacterOverlay->ScoreSpacerText;

	if (bHUDValid)
	{
		BlasterHUD->CharacterOverlay->RedTeamScoreText->SetVisibility(ESlateVisibility::Hidden);
		BlasterHUD->CharacterOverlay->BlueTeamScoreText->SetVisibility(ESlateVisibility::Hidden);
		BlasterHUD->CharacterOverlay->ScoreSpacerText->SetVisibility(ESlateVisibility::Hidden);
	}
}

void ABlasterPlayerController::InitTeamScores()
{
	BlasterHUD = BlasterHUD == nullptr ? Cast<ABlasterHUD>(GetHUD()) : BlasterHUD;
	const bool bHUDValid =
		BlasterHUD &&
		BlasterHUD->CharacterOverlay &&
		BlasterHUD->CharacterOverlay->RedTeamScoreText &&
		BlasterHUD->CharacterOverlay->BlueTeamScoreText &&
		BlasterHUD->CharacterOverlay->ScoreSpacerText;

	if (bHUDValid)
	{
		BlasterHUD->CharacterOverlay->RedTeamScoreText->SetVisibility(ESlateVisibility::Visible);
		BlasterHUD->CharacterOverlay->BlueTeamScoreText->SetVisibility(ESlateVisibility::Visible);
		BlasterHUD->CharacterOverlay->ScoreSpacerText->SetVisibility(ESlateVisibility::Visible);
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

void ABlasterPlayerController::HighPingWarning()
{
	BlasterHUD = BlasterHUD == nullptr ? Cast<ABlasterHUD>(GetHUD()) : BlasterHUD;
	const bool bHUDValid =
		BlasterHUD &&
		BlasterHUD->CharacterOverlay &&
		BlasterHUD->CharacterOverlay->HighPingImage &&
		BlasterHUD->CharacterOverlay->HighPingAnimation;

	if (bHUDValid)
	{
		BlasterHUD->CharacterOverlay->HighPingImage->SetOpacity(1.f);
		BlasterHUD->CharacterOverlay->PlayAnimation(BlasterHUD->CharacterOverlay->HighPingAnimation, 0.f, 5.f);
	}
}

void ABlasterPlayerController::StopHighPingWarning()
{
	BlasterHUD = BlasterHUD == nullptr ? Cast<ABlasterHUD>(GetHUD()) : BlasterHUD;
	const bool bHUDValid =
		BlasterHUD &&
		BlasterHUD->CharacterOverlay &&
		BlasterHUD->CharacterOverlay->HighPingImage &&
		BlasterHUD->CharacterOverlay->HighPingAnimation;

	if (bHUDValid)
	{
		BlasterHUD->CharacterOverlay->HighPingImage->SetOpacity(0.f);
		if (BlasterHUD->CharacterOverlay->IsAnimationPlaying(BlasterHUD->CharacterOverlay->HighPingAnimation))
		{
			BlasterHUD->CharacterOverlay->StopAnimation(BlasterHUD->CharacterOverlay->HighPingAnimation);	
		}
	}
}

void ABlasterPlayerController::CheckPing(float DeltaTime)
{
	HighPingRunningTime += DeltaTime;
	if (HighPingRunningTime > CheckPingFrequency)
	{
		ABlasterPlayerState* BlasterPlayerState = GetPlayerState<ABlasterPlayerState>();
		if (BlasterPlayerState)
		{
			float PingInMilisec = BlasterPlayerState->GetPingInMilliseconds();
			if (PingInMilisec > HighPingThreshold)
			{
				HighPingWarning();
				PingAnimationRunningTime = 0.f;
				ServerReportPingStatus(true);
			}
			else
			{
				ServerReportPingStatus(false);
			}
		}
		HighPingRunningTime = 0.f;
	}
	const bool bHighPingAnimationPlaying = BlasterHUD && BlasterHUD->CharacterOverlay && BlasterHUD->CharacterOverlay->HighPingAnimation && BlasterHUD->CharacterOverlay->IsAnimationPlaying(BlasterHUD->CharacterOverlay->HighPingAnimation);
	if (bHighPingAnimationPlaying)
	{
		PingAnimationRunningTime += DeltaTime;
		if (PingAnimationRunningTime > HighPingDuration)
		{
			StopHighPingWarning();
		}
	}
}

void ABlasterPlayerController::ShowReturnToMainMenu()
{
	if (ReturnToMainMenuWidgetClass == nullptr)
	{
		return;
	}
	if (ReturnToMainMenuWidget == nullptr)
	{
		ReturnToMainMenuWidget = CreateWidget<UReturnToMainMenuWidget>(this, ReturnToMainMenuWidgetClass);
	}
	if (ReturnToMainMenuWidget)
	{
		bReturnToMainMenuOpen = !bReturnToMainMenuOpen;
		if (bReturnToMainMenuOpen)
		{
			ReturnToMainMenuWidget->MenuSetup();
		}
		else
		{
			ReturnToMainMenuWidget->MenuTearDown();
		}
	}
}

void ABlasterPlayerController::ClientElimAnnouncement(const APlayerState* Attacker, const APlayerState* Victim)
{
	const APlayerState* SelfState = GetPlayerState<APlayerState>();
	if (SelfState && Attacker && Victim)
	{
		BlasterHUD = BlasterHUD == nullptr ? GetHUD<ABlasterHUD>() : BlasterHUD;
		if (BlasterHUD)
		{
			if (Attacker == SelfState && Victim != SelfState)
			{
				BlasterHUD->AddElimAnnouncement(Attacker->GetPlayerName(), Victim->GetPlayerName());
				return;
			}
			if (Victim == SelfState && Attacker != SelfState)
			{
				BlasterHUD->AddElimAnnouncement(Attacker->GetPlayerName(), "you");
				return;
			}
			if (Attacker == Victim && Attacker == SelfState)
			{
				BlasterHUD->AddElimAnnouncement("You", "yourself");
				return;
			}
			if (Attacker == Victim && Attacker != SelfState)
			{
				BlasterHUD->AddElimAnnouncement(Attacker->GetPlayerName(), "themselves");
				return;
			}
			BlasterHUD->AddElimAnnouncement(Attacker->GetPlayerName(), Victim->GetPlayerName());
		}
	}
}

void ABlasterPlayerController::ClientReportServerTime_Implementation(float TimeOfClientRequest,
                                                                     float TimeServerReceivedClientRequest)
{
	const float RoundTripTime = GetWorld()->GetTimeSeconds() - TimeOfClientRequest;
	SingleTripTime =  0.5f * RoundTripTime;
	const float CurrentServerTime = TimeServerReceivedClientRequest + SingleTripTime;
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
