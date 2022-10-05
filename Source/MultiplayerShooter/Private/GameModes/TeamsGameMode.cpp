// Fill out your copyright notice in the Description page of Project Settings.

#include "GameModes/TeamsGameMode.h"

#include "Character/BlasterPlayerController.h"
#include "GameStates/BlasterGameState.h"
#include "Kismet/GameplayStatics.h"

ATeamsGameMode::ATeamsGameMode()
{
	bTeamsMatch = true;
}

void ATeamsGameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);
	
	if (ABlasterGameState* BGameState = Cast<ABlasterGameState>(UGameplayStatics::GetGameState(this)))
	{
		ABlasterPlayerState* BPState = NewPlayer->GetPlayerState<ABlasterPlayerState>();
		if (BPState && BPState->GetTeam() == ETeam::ET_NoTeam)
		{
			if (BGameState->BlueTeam.Num() >= BGameState->RedTeam.Num())
			{
				BGameState->RedTeam.AddUnique(BPState);
				BPState->SetTeam(ETeam::ET_RedTeam);
			}
			else
			{
				BGameState->BlueTeam.AddUnique(BPState);
				BPState->SetTeam(ETeam::ET_BlueTeam);
			}
		}
	}
}

void ATeamsGameMode::Logout(AController* Exiting)
{
	Super::Logout(Exiting);

	if (ABlasterGameState* BGameState = Cast<ABlasterGameState>(UGameplayStatics::GetGameState(this)))
	{
		if (ABlasterPlayerState* BPState = Exiting->GetPlayerState<ABlasterPlayerState>())
		{
			if (BGameState->RedTeam.Contains(BPState))
			{
				BGameState->RedTeam.Remove(BPState);
			}
			if (BGameState->BlueTeam.Contains(BPState))
			{
				BGameState->BlueTeam.Remove(BPState);
			}
		}
	}
}

float ATeamsGameMode::CalculateDamage(const AController* VictimController,
	const AController* AttackerController, float BaseDamage)
{
	if (AttackerController == nullptr || VictimController == nullptr)
	{
		return BaseDamage;
	}
	
	ABlasterPlayerState* AttackerPState = AttackerController->GetPlayerState<ABlasterPlayerState>();
	ABlasterPlayerState* VictimPState = VictimController->GetPlayerState<ABlasterPlayerState>();
	if (AttackerPState == nullptr || VictimPState == nullptr)
	{
		return BaseDamage;
	}
	if (VictimPState == AttackerPState)
	{
		return BaseDamage;
	}
	if (AttackerPState->GetTeam() == VictimPState->GetTeam())
	{
		return 0.f;
	}
	return BaseDamage;
}

void ATeamsGameMode::PlayerEliminated(ABlasterCharacter* EliminatedCharacter,
	ABlasterPlayerController* VictimController, ABlasterPlayerController* AttackerController)
{
	Super::PlayerEliminated(EliminatedCharacter, VictimController, AttackerController);
	ABlasterGameState* BGameState = GetGameState<ABlasterGameState>();
	ABlasterPlayerState* AttackerPState = AttackerController->GetPlayerState<ABlasterPlayerState>();
	if (BGameState && AttackerPState)
	{
		if (AttackerPState->GetTeam() == ETeam::ET_BlueTeam)
		{
			BGameState->BlueTeamScores();
		}
		if (AttackerPState->GetTeam() == ETeam::ET_RedTeam)
		{
			BGameState->RedTeamScores();
		}
	}
}

void ATeamsGameMode::HandleMatchHasStarted()
{
	Super::HandleMatchHasStarted();
	if (ABlasterGameState* BGameState = Cast<ABlasterGameState>(UGameplayStatics::GetGameState(this)))
	{
		for (auto& PState : BGameState->PlayerArray)
		{
			ABlasterPlayerState* BPState = Cast<ABlasterPlayerState>(PState.Get());
			if (BPState && BPState->GetTeam() == ETeam::ET_NoTeam)
			{
				if (BGameState->BlueTeam.Num() >= BGameState->RedTeam.Num())
				{
					BGameState->RedTeam.AddUnique(BPState);
					BPState->SetTeam(ETeam::ET_RedTeam);
				}
				else
				{
					BGameState->BlueTeam.AddUnique(BPState);
					BPState->SetTeam(ETeam::ET_BlueTeam);
				}
			}
		}
	}
}
