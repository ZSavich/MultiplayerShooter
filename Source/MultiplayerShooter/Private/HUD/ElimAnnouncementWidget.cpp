// Fill out your copyright notice in the Description page of Project Settings.

#include "HUD/ElimAnnouncementWidget.h"

void UElimAnnouncementWidget::SetElimAnnouncementText(const FString& AttackerName, const FString& VictimName)
{
	FString ElimAnnouncementText = FString::Printf(TEXT("%s elimmed %s!"), *AttackerName, *VictimName);
	if (AnnouncementText)
	{
		AnnouncementText->SetText(FText::FromString(ElimAnnouncementText));
	}
}
