// Fill out your copyright notice in the Description page of Project Settings.


#include "GameModes/LobbyGameMode.h"

#include "GameFramework/GameStateBase.h"

void ALobbyGameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);

	const int32 NumberOfPlayers = GameState.Get()->PlayerArray.Num();
	if (NumberOfPlayers == 2)
	{
		bUseSeamlessTravel = true;
		if (UWorld* World = GetWorld())
		{
			World->ServerTravel(FString("/Game/Maps/L_General_P?listen"));
		}
	}
}
