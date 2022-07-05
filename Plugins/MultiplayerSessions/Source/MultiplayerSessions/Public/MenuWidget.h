// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "MenuWidget.generated.h"

class UButton;
class UMultiplayerSessionSubsystem;

UCLASS()
class MULTIPLAYERSESSIONS_API UMenuWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual bool Initialize() override;
	virtual void OnLevelRemovedFromWorld(ULevel* InLevel, UWorld* InWorld) override;

	UFUNCTION(BlueprintCallable)
	void MenuSetup(const int32 NumberPublicConnections = 4, const FString TypeOfMatch = "FreeForAll", const FString LobbyPath = FString(TEXT("/Game/ThirdPerson/Maps/L_Lobby_P")));
	void MenuTearDown();

protected:
	UFUNCTION()
	void HandleOnHostButtonClicked();

	UFUNCTION()
	void HandleOnJoinButtonClicked();

	UFUNCTION()
	void HandleOnCreateSession(bool bWasSuccessful);
	void HandleOnFindSessions(const TArray<FOnlineSessionSearchResult>& SearchResults, bool bWasSuccessful);
	void HandleOnJoinSession(const EOnJoinSessionCompleteResult::Type Result);
	UFUNCTION()
	void HandleOnStartSession(bool bWasSuccessful);
	UFUNCTION()
	void HandleOnDestroySession(bool bWasSuccessful);

private:
	int32 NumPublicConnections{4};
	FString MatchType{"FreeForAll"};
	FString PathToLobby{TEXT("")};

	UPROPERTY(meta = (BindWidget))
	UButton* HostButton{nullptr};

	UPROPERTY(meta = (BindWidget))
	UButton* JoinButton{nullptr};

	UPROPERTY()
	UMultiplayerSessionSubsystem* MultiplayerSessionSubsystem;
};
