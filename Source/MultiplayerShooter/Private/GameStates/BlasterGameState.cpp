// Fill out your copyright notice in the Description page of Project Settings.


#include "GameStates/BlasterGameState.h"

#include "Character/BlasterPlayerController.h"
#include "Net/UnrealNetwork.h"

void ABlasterGameState::GetLifetimeReplicatedProps(TArray< FLifetimeProperty >& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ABlasterGameState, TopScoringPlayers);
	DOREPLIFETIME(ABlasterGameState, RedTeamScore);
	DOREPLIFETIME(ABlasterGameState, BlueTeamScore);
}

void ABlasterGameState::UpdateTopScore(ABlasterPlayerState* ScoringPlayer)
{
	if (ScoringPlayer == nullptr)
	{
		return;
	}
	
	if (TopScoringPlayers.Num() == 0)
	{
		TopScoringPlayers.Add(ScoringPlayer);
		TopScore = ScoringPlayer->GetScore();
	}
	else if (ScoringPlayer->GetScore() == TopScore)
	{
		TopScoringPlayers.AddUnique(ScoringPlayer);
	}
	else if (ScoringPlayer->GetScore() > TopScore)
	{
		TopScoringPlayers.Empty();
		TopScoringPlayers.AddUnique(ScoringPlayer);
		TopScore = ScoringPlayer->GetScore();
	}
}

void ABlasterGameState::OnRep_RedTeamScore()
{
	if (ABlasterPlayerController* BController = Cast<ABlasterPlayerController>(GetWorld()->GetFirstPlayerController()))
	{
		BController->SetHUDRedTeamScore(RedTeamScore);
	}
}

void ABlasterGameState::OnRep_BlueTeamScore()
{
	if (ABlasterPlayerController* BController = Cast<ABlasterPlayerController>(GetWorld()->GetFirstPlayerController()))
	{
		BController->SetHUDBlueTeamScore(BlueTeamScore);
	}
}

void ABlasterGameState::RedTeamScores()
{
	++RedTeamScore;
	if (ABlasterPlayerController* BController = Cast<ABlasterPlayerController>(GetWorld()->GetFirstPlayerController()))
	{
		BController->SetHUDRedTeamScore(RedTeamScore);
	}
}

void ABlasterGameState::BlueTeamScores()
{
	++BlueTeamScore;
	if (ABlasterPlayerController* BController = Cast<ABlasterPlayerController>(GetWorld()->GetFirstPlayerController()))
	{
		BController->SetHUDBlueTeamScore(BlueTeamScore);
	}
}
