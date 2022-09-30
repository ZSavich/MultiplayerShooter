// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "../../../../Plugins/MultiplayerSessions/Source/MultiplayerSessions/Public/MultiplayerSessionSubsystem.h"
#include "Blueprint/UserWidget.h"
#include "Character/BlasterPlayerController.h"
#include "Components/Button.h"
#include "ReturnToMainMenuWidget.generated.h"

UCLASS()
class MULTIPLAYERSHOOTER_API UReturnToMainMenuWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	void MenuSetup();
	void MenuTearDown();

protected:
	void OnDestroySession(bool bWasSuccessful);
	void OnPlayerLeftGame();
	
private:
	UPROPERTY()
	ABlasterPlayerController* PlayerController;

	UPROPERTY(meta = (BindWidget))
	UButton* ReturnButton;

	UPROPERTY()
	UMultiplayerSessionSubsystem* MultiplayerSessionSubsystem;

	UFUNCTION()
	void ReturnButtonClicked();
};
