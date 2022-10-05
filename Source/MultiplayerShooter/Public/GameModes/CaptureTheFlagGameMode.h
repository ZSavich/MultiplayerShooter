// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameModes/TeamsGameMode.h"
#include "Weapons/Flag.h"
#include "Zones/FlagZone.h"
#include "CaptureTheFlagGameMode.generated.h"

UCLASS()
class MULTIPLAYERSHOOTER_API ACaptureTheFlagGameMode : public ATeamsGameMode
{
	GENERATED_BODY()

public:
	virtual void PlayerEliminated(ABlasterCharacter* EliminatedCharacter, ABlasterPlayerController* VictimController, ABlasterPlayerController* AttackerController) override;
	void FlagCaptured(const AFlag* Flag, const AFlagZone* FlagZone);
};
