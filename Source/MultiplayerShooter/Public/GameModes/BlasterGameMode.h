// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "PlayerStates/BlasterPlayerState.h"
#include "BlasterGameMode.generated.h"

class ABlasterCharacter;
class ABlasterPlayerController;

namespace MatchState
{
	extern MULTIPLAYERSHOOTER_API const FName Cooldown;
}

UCLASS()
class MULTIPLAYERSHOOTER_API ABlasterGameMode : public AGameMode
{
	GENERATED_BODY()

private:
	float CountdownTime = 0.f;
	
public:
	UPROPERTY(EditDefaultsOnly, Category = "Time")
	float WarmupTime = 10.f;

	UPROPERTY(EditDefaultsOnly, Category = "Time")
	float MatchTime = 120.f;

	UPROPERTY(EditDefaultsOnly, Category = "Time")
	float CooldownTime = 10.f;
	
	float LevelStartingTime = 0.f;

	bool bTeamsMatch = false;

protected:
	virtual void BeginPlay() override;
	virtual void OnMatchStateSet() override;
	
public:
	ABlasterGameMode();
	virtual void PlayerEliminated(ABlasterCharacter* EliminatedCharacter, ABlasterPlayerController* VictimController, ABlasterPlayerController* AttackerController);
	virtual void RequestRespawn(AActor* EliminatedCharacter, AController* EliminatedController);
	virtual void Tick(float DeltaSeconds) override;
	virtual float CalculateDamage(const AController* VictimController, const AController* AttackerController, float BaseDamage);

	void OnPlayerLeftGame(ABlasterPlayerState* PlayerLeaving);

	FORCEINLINE float GetCountdownTime() const { return CooldownTime; }
};
