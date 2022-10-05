#pragma once

UENUM(BlueprintType)
enum class ETeam : uint8
{
	ET_RedTeam	UMETA(DisplayText = "Red Team"),
	ET_BlueTeam	UMETA(DisplayText = "Blue Team"),
	ET_NoTeam	UMETA(DisplayText = "No Team"),

	ET_MAX		UMETA(Hidden)
};