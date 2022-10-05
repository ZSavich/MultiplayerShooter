// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameModes/BlasterGameMode.h"
#include "TeamsGameMode.generated.h"

UCLASS()
class MULTIPLAYERSHOOTER_API ATeamsGameMode : public ABlasterGameMode
{
	GENERATED_BODY()

public:
	ATeamsGameMode();
	
	virtual void PostLogin(APlayerController* NewPlayer) override;
	virtual void Logout(AController* Exiting) override;
	virtual float CalculateDamage(const AController* VictimController, const AController* AttackerController, float BaseDamage) override;
	virtual void PlayerEliminated(ABlasterCharacter* EliminatedCharacter, ABlasterPlayerController* VictimController, ABlasterPlayerController* AttackerController) override;
	
protected:
	virtual void HandleMatchHasStarted() override;
};
