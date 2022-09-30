// Fill out your copyright notice in the Description page of Project Settings.

#include "HUD/BlasterHUD.h"

#include "Blueprint/WidgetLayoutLibrary.h"
#include "Character/BlasterPlayerController.h"
#include "Components/CanvasPanelSlot.h"
#include "HUD/AnnouncementWidget.h"
#include "HUD/CharacterOverlayWidget.h"
#include "HUD/ElimAnnouncementWidget.h"

void ABlasterHUD::BeginPlay()
{
	Super::BeginPlay();
}

void ABlasterHUD::DrawHUD()
{
	Super::DrawHUD();

	FVector2D ViewportSize;
	if (GEngine)
	{
		GEngine->GameViewport->GetViewportSize(ViewportSize);
		FVector2D ViewportCenter(
			ViewportSize.X / 2.f,
			ViewportSize.Y / 2.f
		);
		const float SpreadScale = CrosshairSpreadMax * HUDPackage.CrosshairSpread;
		
		if (HUDPackage.CrosshairCenter)
		{
			const FVector2D Spread(0.f, 0.f);
			DrawCrosshair(HUDPackage.CrosshairCenter, ViewportCenter, Spread, HUDPackage.CrosshairColor);
		}
		if (HUDPackage.CrosshairLeft)
		{
			const FVector2D Spread(-SpreadScale, 0.f);
			DrawCrosshair(HUDPackage.CrosshairLeft, ViewportCenter, Spread, HUDPackage.CrosshairColor);
		}
		if (HUDPackage.CrosshairRight)
		{
			const FVector2D Spread(SpreadScale, 0.f);
			DrawCrosshair(HUDPackage.CrosshairRight, ViewportCenter, Spread, HUDPackage.CrosshairColor);
		}
		if (HUDPackage.CrosshairTop)
		{
			const FVector2D Spread(0.f, -SpreadScale);
			DrawCrosshair(HUDPackage.CrosshairTop, ViewportCenter, Spread, HUDPackage.CrosshairColor);
		}
		if (HUDPackage.CrosshairBottom)
		{
			const FVector2D Spread(0.f, SpreadScale);
			DrawCrosshair(HUDPackage.CrosshairBottom, ViewportCenter, Spread, HUDPackage.CrosshairColor);
		}
	}
}

void ABlasterHUD::DrawCrosshair(UTexture2D* Texture, FVector2D ViewportCenter, FVector2D Spread, FLinearColor Color)
{
	const float TextureWidth = Texture->GetSizeX();
	const float TextureHeight = Texture->GetSizeY();
	const FVector2D TextureDrawPoint(
		ViewportCenter.X - (TextureWidth / 2.f) + Spread.X,
		ViewportCenter.Y - (TextureHeight / 2.f) + Spread.Y
	);

	DrawTexture(
		Texture,
		TextureDrawPoint.X,
		TextureDrawPoint.Y,
		TextureWidth,
		TextureHeight,
		0.f,
		0.f,
		1.f,
		1.f,
		Color
	);
}

void ABlasterHUD::AddCharacterOverlay()
{
	APlayerController* PlayerController = GetOwningPlayerController();
	if (PlayerController && CharacterOverlayClass)
	{
		CharacterOverlay = CreateWidget<UCharacterOverlayWidget>(PlayerController, CharacterOverlayClass);
		CharacterOverlay->AddToViewport();
	}
}

void ABlasterHUD::AddAnnouncementWidget()
{
	APlayerController* PlayerController = GetOwningPlayerController();
	if (PlayerController && AnnouncementClass)
	{
		AnnouncementWidget = CreateWidget<UAnnouncementWidget>(PlayerController, AnnouncementClass);
		AnnouncementWidget->AddToViewport();
	}
}

void ABlasterHUD::AddElimAnnouncement(const FString& AttackerName, const FString& VictimName)
{
	OwningPlayer = OwningPlayer == nullptr ? Cast<ABlasterPlayerController>(GetOwningPlayerController()) : OwningPlayer;
	if (OwningPlayer && ElimAnnouncementWidgetClass)
	{
		if (UElimAnnouncementWidget* ElimAnnouncementWidget = CreateWidget<UElimAnnouncementWidget>(OwningPlayer, ElimAnnouncementWidgetClass))
		{
			ElimAnnouncementWidget->SetElimAnnouncementText(AttackerName, VictimName);
			ElimAnnouncementWidget->AddToViewport();

			ElimMessages.Add(ElimAnnouncementWidget);
			for (auto& Msg : ElimMessages)
			{
				if (Msg && Msg->AnnouncementBox)
				{
					if (UCanvasPanelSlot* CanvasSlot = UWidgetLayoutLibrary::SlotAsCanvasSlot(Msg->AnnouncementBox))
					{
						FVector2D Position = CanvasSlot->GetPosition();
						FVector2D NewPosition(CanvasSlot->GetPosition().X, Position.Y - CanvasSlot->GetSize().Y);
						CanvasSlot->SetPosition(NewPosition);
					}
				}
			}

			FTimerHandle ElimTimerHandle;
			FTimerDelegate ElimTimerDelegate;
			ElimTimerDelegate.BindUFunction(this, FName("ElimAnnouncementTimerExpired"), ElimAnnouncementWidget);

			GetWorldTimerManager().SetTimer(ElimTimerHandle, ElimTimerDelegate, ElimAnnouncementTime, false);
		}
	}
}

void ABlasterHUD::ElimAnnouncementTimerExpired(UElimAnnouncementWidget* MsgToElim)
{
	if (MsgToElim)
	{
		MsgToElim->RemoveFromParent();
	}
}