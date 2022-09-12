// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Character/BlasterCharacter.h"
#include "Components/ActorComponent.h"
#include "BuffComponent.generated.h"

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class MULTIPLAYERSHOOTER_API UBuffComponent : public UActorComponent
{
	GENERATED_BODY()

private:
	UPROPERTY()
	ABlasterCharacter* Character;

	/* Healing */
	bool bHealing = false;
	float HealingRate = 0.f;
	float AmountToHeal = 0.f;
	
	/* Shield Buff */
	bool bReplenishingShield = false;
	float ShieldReplenishRate = 0.f;
	float ShieldReplenishAmount = 0.f;

	/* Speed Buff */
	FTimerHandle SpeedBuffTimer;
	float InitialBaseSpeed;
	float InitialCrouchSpeed;

	/* Jump Buff */
	FTimerHandle JumpBuffTimer;
	float InitialJumpVelocity;
	
public:	
	UBuffComponent();
	friend ABlasterCharacter;

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	void SetInitialSpeeds(float BaseSpeed, float CrouchSpeed);
	void SetInitialJumpVelocity(float Velocity);
	
	/* Healing */
	void Heal(float HealAmount, float HealingTime);

	/* Speed Buff */
	void BuffSpeed(float BuffBaseSpeed, float BuffCrouchSpeed, float BuffTime);

	/* Jump Buff */
	void BuffJump(float BuffJumpVelocity, float BuffTime);

	/* Shield Buff */
	void ReplenishShield(float ShieldAmount, float ReplenishTime);
	
protected:
	virtual void BeginPlay() override;
	
	void HealRampUp(float DeltaSeconds);
	void ShieldRampUp(float DeltaSeconds);

private:
	UFUNCTION()
	void ResetSpeeds();

	UFUNCTION()
	void ResetJump();

	UFUNCTION(NetMulticast, Reliable)
	void MulticastSpeedBuff(float BaseSpeed, float CrouchSpeed);

	UFUNCTION(NetMulticast, Reliable)
	void MulticastJumpBuff(float JumpVelocity);
};
