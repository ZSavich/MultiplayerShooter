// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "BlasterGameMode.generated.h"

class ABlasterCharacter;
class ABlasterPlayerController;

UCLASS()
class MULTIPLAYERSHOOTER_API ABlasterGameMode : public AGameMode
{
	GENERATED_BODY()

public:
	virtual void PlayerEliminated(ABlasterCharacter* EliminatedCharacter, ABlasterPlayerController* VictimController, ABlasterPlayerController* AttackerController);
	virtual void RequestRespawn(AActor* EliminatedCharacter, AController* EliminatedController);
};
