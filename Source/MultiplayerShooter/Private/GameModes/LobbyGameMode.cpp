// Fill out your copyright notice in the Description page of Project Settings.


#include "GameModes/LobbyGameMode.h"

#include "MultiplayerSessionSubsystem.h"
#include "GameFramework/GameStateBase.h"

void ALobbyGameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);

	const int32 NumberOfPlayers = GameState.Get()->PlayerArray.Num();

	if (UGameInstance* GameInstance = GetGameInstance())
	{
		UMultiplayerSessionSubsystem* Subsystem = GameInstance->GetSubsystem<UMultiplayerSessionSubsystem>();
		check(Subsystem);
		
		if (NumberOfPlayers == Subsystem->DesiredNumPublicConnections)
		{
			bUseSeamlessTravel = true;
			FString MatchType = Subsystem->DesiredMatchType;
			if (UWorld* World = GetWorld())
			{
				if (MatchType == "FreeForAll")
				{
					World->ServerTravel(FString("/Game/Maps/L_General_P?listen"));
				}
				else if (MatchType == "Teams")
				{
					World->ServerTravel(FString("/Game/Maps/L_Teams_P?listen"));
				}
				else if (MatchType == "CaptureTheFlag")
				{
					World->ServerTravel(FString("/Game/Maps/L_CaptureTheFlag_P?listen"));
				}
			}
		}
	}
}
