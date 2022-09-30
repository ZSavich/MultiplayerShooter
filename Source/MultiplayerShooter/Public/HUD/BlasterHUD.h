// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "BlasterHUD.generated.h"

class UCharacterOverlayWidget;
class UAnnouncementWidget;
class UElimAnnouncementWidget;
class ABlasterPlayerController;

USTRUCT()
struct FHUDPackage
{
	GENERATED_BODY()
public:
	UPROPERTY()
	UTexture2D* CrosshairCenter;

	UPROPERTY()
	UTexture2D* CrosshairLeft;

	UPROPERTY()
	UTexture2D* CrosshairRight;

	UPROPERTY()
	UTexture2D* CrosshairTop;

	UPROPERTY()
	UTexture2D* CrosshairBottom;

	float CrosshairSpread;
	FLinearColor CrosshairColor;
};

UCLASS()
class MULTIPLAYERSHOOTER_API ABlasterHUD : public AHUD
{
	GENERATED_BODY()
	
private:
	UPROPERTY(EditAnywhere)
	TSubclassOf<UElimAnnouncementWidget> ElimAnnouncementWidgetClass;

	UPROPERTY(EditAnywhere)
	float ElimAnnouncementTime = 2.5;

	UPROPERTY()
	ABlasterPlayerController* OwningPlayer;
	
	FHUDPackage HUDPackage;
	
	float CrosshairSpreadMax = 16.f;

public:
	UPROPERTY(EditDefaultsOnly, Category = "Widgets")
	TSubclassOf<UCharacterOverlayWidget> CharacterOverlayClass;

	UPROPERTY()
	UCharacterOverlayWidget* CharacterOverlay;

	UPROPERTY(EditDefaultsOnly, Category = "Widgets")
	TSubclassOf<UAnnouncementWidget> AnnouncementClass;

	UPROPERTY()
	UAnnouncementWidget* AnnouncementWidget;

private:
	void DrawCrosshair(UTexture2D* Texture, FVector2D ViewportCenter, FVector2D Spread, FLinearColor Color);

	UFUNCTION()
	void ElimAnnouncementTimerExpired(UElimAnnouncementWidget* MsgToElim);

protected:
	virtual void BeginPlay() override;

	UPROPERTY()
	TArray<UElimAnnouncementWidget*> ElimMessages;
	
public:
	virtual void DrawHUD() override;
	
	void AddCharacterOverlay();
	void AddAnnouncementWidget();
	void AddElimAnnouncement(const FString& AttackerName, const FString& VictimName);

	FORCEINLINE void SetHUDPackage(const FHUDPackage& Package) { HUDPackage = Package; }

};
