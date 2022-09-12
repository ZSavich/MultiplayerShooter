// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/BuffComponent.h"

#include "GameFramework/CharacterMovementComponent.h"

UBuffComponent::UBuffComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UBuffComponent::BeginPlay()
{
	Super::BeginPlay();
}

void UBuffComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	HealRampUp(DeltaTime);
	ShieldRampUp(DeltaTime);
}

void UBuffComponent::SetInitialSpeeds(float BaseSpeed, float CrouchSpeed)
{
	InitialBaseSpeed = BaseSpeed;
	InitialCrouchSpeed = CrouchSpeed;
}

void UBuffComponent::SetInitialJumpVelocity(float Velocity)
{
	InitialJumpVelocity = Velocity;
}

void UBuffComponent::HealRampUp(float DeltaSeconds)
{
	if (!bHealing || !Character || Character->IsEliminated())
	{
		return;
	}
	const float HealThisFrame = HealingRate * DeltaSeconds;
	Character->SetHealth(FMath::Clamp(Character->GetHealth() + HealThisFrame, 0.f, Character->GetMaxHealth()));
	Character->UpdateHUDHealth();

	AmountToHeal -= HealThisFrame;
	if (AmountToHeal <= 0.f || Character->GetHealth() >= Character->GetMaxHealth())
	{
		bHealing = false;
		AmountToHeal = 0.f;
	}
}

void UBuffComponent::ShieldRampUp(float DeltaSeconds)
{
	if (!bReplenishingShield || !Character || Character->IsEliminated())
    {
    	return;
    }
    const float ShieldThisFrame = ShieldReplenishRate * DeltaSeconds;
    Character->SetShield(FMath::Clamp(Character->GetShield() + ShieldThisFrame, 0.f, Character->GetMaxShield()));
    Character->UpdateHUDShield();

    ShieldReplenishAmount -= ShieldThisFrame;
    if (ShieldReplenishAmount <= 0.f || Character->GetShield() >= Character->GetMaxShield())
    {
    	bReplenishingShield = false;
    	ShieldReplenishAmount = 0.f;
    }
}

void UBuffComponent::Heal(float HealAmount, float HealingTime)
{
	bHealing = true;
	HealingRate = HealAmount / HealingTime;
	AmountToHeal += HealAmount;
}

void UBuffComponent::BuffSpeed(float BuffBaseSpeed, float BuffCrouchSpeed, float BuffTime)
{
	if (Character)
	{
		Character->GetWorldTimerManager().SetTimer(SpeedBuffTimer, this, &UBuffComponent::ResetSpeeds, BuffTime);
		if (Character->GetCharacterMovement())
		{
			Character->GetCharacterMovement()->MaxWalkSpeed = BuffBaseSpeed;
			Character->GetCharacterMovement()->MaxWalkSpeedCrouched = BuffCrouchSpeed;
		}
		MulticastSpeedBuff(BuffBaseSpeed, BuffCrouchSpeed);
	}
}

void UBuffComponent::BuffJump(float BuffJumpVelocity, float BuffTime)
{
	if (Character)
	{
		Character->GetWorldTimerManager().SetTimer(JumpBuffTimer, this, &UBuffComponent::ResetJump, BuffTime);
		if (Character->GetCharacterMovement())
		{
			Character->GetCharacterMovement()->JumpZVelocity = BuffJumpVelocity;
		}
		MulticastJumpBuff(BuffJumpVelocity);
	}
}

void UBuffComponent::ReplenishShield(float ShieldAmount, float ReplenishTime)
{
	bReplenishingShield = true;
	ShieldReplenishRate = ShieldAmount / ReplenishTime;
	ShieldReplenishAmount += ShieldAmount;
}

void UBuffComponent::ResetSpeeds()
{
	if (!Character || !Character->GetCharacterMovement())
	{
		return;
	}
	Character->GetCharacterMovement()->MaxWalkSpeed = InitialBaseSpeed;
	Character->GetCharacterMovement()->MaxWalkSpeedCrouched = InitialCrouchSpeed;
}

void UBuffComponent::ResetJump()
{
	if (!Character || !Character->GetCharacterMovement())
	{
		return;
	}
	Character->GetCharacterMovement()->JumpZVelocity = InitialJumpVelocity;
	MulticastJumpBuff(InitialJumpVelocity);
}

void UBuffComponent::MulticastSpeedBuff_Implementation(float BaseSpeed, float CrouchSpeed)
{
	if (!Character || !Character->GetCharacterMovement())
	{
		return;
	}
	Character->GetCharacterMovement()->MaxWalkSpeed = BaseSpeed;
	Character->GetCharacterMovement()->MaxWalkSpeedCrouched = CrouchSpeed;
}

void UBuffComponent::MulticastJumpBuff_Implementation(float JumpVelocity)
{
	if (!Character || !Character->GetCharacterMovement())
	{
		return;
	}
	Character->GetCharacterMovement()->JumpZVelocity = JumpVelocity;
}
