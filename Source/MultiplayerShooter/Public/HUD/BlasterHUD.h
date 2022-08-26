// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "BlasterHUD.generated.h"

class UCharacterOverlayWidget;

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
	FHUDPackage HUDPackage;
	float CrosshairSpreadMax = 16.f;
	
	void DrawCrosshair(UTexture2D* Texture, FVector2D ViewportCenter, FVector2D Spread, FLinearColor Color);

public:
	UPROPERTY(EditDefaultsOnly, Category = "Widgets")
	TSubclassOf<UCharacterOverlayWidget> CharacterOverlayClass;

	UPROPERTY()
	UCharacterOverlayWidget* CharacterOverlay;
	
	virtual void DrawHUD() override;

	FORCEINLINE void SetHUDPackage(const FHUDPackage& Package) { HUDPackage = Package; }

protected:
	virtual void BeginPlay() override;
	
	void AddCharacterOverlay();
};
