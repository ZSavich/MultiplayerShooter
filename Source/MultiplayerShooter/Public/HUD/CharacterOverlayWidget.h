// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/Image.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "CharacterOverlayWidget.generated.h"

/**
 * 
 */
UCLASS()
class MULTIPLAYERSHOOTER_API UCharacterOverlayWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UPROPERTY(meta = (BindWidget))
	UProgressBar* HealthBar{nullptr};

	UPROPERTY(meta = (BindWidget))
	UTextBlock* HealthText{nullptr};

	UPROPERTY(meta = (BindWidget))
	UProgressBar* ShieldBar{nullptr};

	UPROPERTY(meta = (BindWidget))
	UTextBlock* ShieldText{nullptr};

	UPROPERTY(meta = (BindWidget))
	UTextBlock* ScoreAmount{nullptr};

	UPROPERTY(meta = (BindWidget))
	UTextBlock* DefeatsAmount{nullptr};

	UPROPERTY(meta = (BindWidget))
	UTextBlock* WeaponAmmoAmount{nullptr};

	UPROPERTY(meta = (BindWidget))
	UTextBlock* CarriedAmmoAmount{nullptr};

	UPROPERTY(meta = (BindWidget))
	UTextBlock* MatchCountdownText{nullptr};

	UPROPERTY(meta = (BindWidget))
	UTextBlock* WeaponTypeText{nullptr};

	UPROPERTY(meta = (BindWidget))
	UTextBlock* GrenadesText;

	UPROPERTY(meta = (BindWidget))
	UImage* HighPingImage;

	UPROPERTY(meta = (BindWidgetAnim), Transient)
	UWidgetAnimation* HighPingAnimation;
};
