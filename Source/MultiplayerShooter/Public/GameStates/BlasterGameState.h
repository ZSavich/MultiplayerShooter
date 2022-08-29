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
	
private:
	float TopScore = 0.f;
	
public:
	UPROPERTY(Replicated)
	TArray<ABlasterPlayerState*> TopScoringPlayers;

public:
	void UpdateTopScore(ABlasterPlayerState* ScoringPlayer);
};
