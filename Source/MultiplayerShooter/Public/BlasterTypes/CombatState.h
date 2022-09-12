#pragma once

UENUM(BlueprintType)
enum class ECombatState : uint8
{
	ECS_Unoccupied		UMETA(DisplayName = "Unoccupied"),
	ECS_Reloading		UMETA(DisplayName = "Reloading"),
	ECS_ThrowingGrenade	UMETA(DisplayName = "Throwing Grenade"),

	ESC_MAX				UMETA(Hidden)
};