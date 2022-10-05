// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BlasterTypes/Team.h"
#include "GameFramework/PlayerState.h"
#include "BlasterPlayerState.generated.h"

class ABlasterCharacter;
class ABlasterPlayerController;

UCLASS()
class MULTIPLAYERSHOOTER_API ABlasterPlayerState : public APlayerState
{
	GENERATED_BODY()

public:
	virtual void OnRep_Score() override;

	UFUNCTION()
	virtual void OnRep_Defeats();

	void AddToScore(const float ScoreAmount);
	void AddToDefeats(const int32 DefeatsAmount);
	void SetTeam(const ETeam TeamToSet);
	
	FORCEINLINE ETeam GetTeam() const { return Team; }

private:
	UPROPERTY()
	ABlasterCharacter* BlasterCharacter;

	UPROPERTY()
	ABlasterPlayerController* BlasterPlayerController;

	UPROPERTY(ReplicatedUsing = OnRep_Defeats)
	float Defeats;

	UPROPERTY(ReplicatedUsing = OnRep_Team)
	ETeam Team = ETeam::ET_NoTeam;

	UFUNCTION()
	void OnRep_Team();
};
