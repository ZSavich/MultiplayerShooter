// Fill out your copyright notice in the Description page of Project Settings.


#include "GameModes/CaptureTheFlagGameMode.h"

#include "GameStates/BlasterGameState.h"

void ACaptureTheFlagGameMode::PlayerEliminated(ABlasterCharacter* EliminatedCharacter,
	ABlasterPlayerController* VictimController, ABlasterPlayerController* AttackerController)
{
	ABlasterGameMode::PlayerEliminated(EliminatedCharacter, VictimController, AttackerController);
}

void ACaptureTheFlagGameMode::FlagCaptured(const AFlag* Flag, const AFlagZone* FlagZone)
{
	bool bValidCapture = Flag->GetTeam() != FlagZone->Team;
	ABlasterGameState* BGameState = GetGameState<ABlasterGameState>();
	if (bValidCapture && BGameState)
	{
		if (FlagZone->Team == ETeam::ET_BlueTeam)
		{
			BGameState->BlueTeamScores();
		}
		else if (FlagZone->Team == ETeam::ET_RedTeam)
		{
			BGameState->RedTeamScores();
		}
	}
}
