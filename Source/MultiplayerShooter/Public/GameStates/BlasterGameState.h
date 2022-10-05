// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameState.h"
#include "PlayerStates/BlasterPlayerState.h"
#include "BlasterGameState.generated.h"

UCLASS()
class MULTIPLAYERSHOOTER_API ABlasterGameState : public AGameState
{
	GENERATED_BODY()
	
public:
	UPROPERTY(Replicated)
	TArray<ABlasterPlayerState*> TopScoringPlayers;

	UPROPERTY()
	TArray<ABlasterPlayerState*> RedTeam;

	UPROPERTY()
	TArray<ABlasterPlayerState*> BlueTeam;

	UPROPERTY(ReplicatedUsing = OnRep_RedTeamScore)
	float RedTeamScore = 0.f;

	UPROPERTY(ReplicatedUsing = OnRep_BlueTeamScore)
	float BlueTeamScore = 0.f;

	UFUNCTION()
	void OnRep_RedTeamScore();

	UFUNCTION()
	void OnRep_BlueTeamScore();

	void RedTeamScores();
	void BlueTeamScores();
	
private:
	float TopScore = 0.f;

public:
	void UpdateTopScore(ABlasterPlayerState* ScoringPlayer);
};
