// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "BlasterTypes/TurningInPlace.h"
#include "BlasterAnimInstance.generated.h"

class ABlasterCharacter;
class AWeaponBase;

UCLASS()
class MULTIPLAYERSHOOTER_API UBlasterAnimInstance : public UAnimInstance
{
	GENERATED_BODY()
	
public:
	virtual void NativeInitializeAnimation() override;
	virtual void NativeUpdateAnimation(float DeltaSeconds) override;

private:
	UPROPERTY(BlueprintReadOnly, Category = "Character", meta = (AllowPrivateAccess = true));
	ABlasterCharacter* BlasterCharacter;

	UPROPERTY(BlueprintReadOnly, Category = "Character", meta = (AllowPrivateAccess = true));
	float Speed;

	UPROPERTY(BlueprintReadOnly, Category = "Character", meta = (AllowPrivateAccess = true));
	bool bIsInAir;

	UPROPERTY(BlueprintReadOnly, Category = "Character", meta = (AllowPrivateAccess = true));
	bool bIsAccelerating;

	UPROPERTY(BlueprintReadOnly, Category = "Character", meta = (AllowPrivateAccess = true));
	bool bWeaponEquipped;

	UPROPERTY(BlueprintReadOnly, Category = "Character", meta = (AllowPrivateAccess = true));
	bool bIsCrouching;

	UPROPERTY(BlueprintReadOnly, Category = "Character", meta = (AllowPrivateAccess = true));
	bool bAiming;

	UPROPERTY(BlueprintReadOnly, Category = "Character", meta = (AllowPrivateAccess = true));
	float YawOffset;

	UPROPERTY(BlueprintReadOnly, Category = "Character", meta = (AllowPrivateAccess = true));
	float Lean;

	UPROPERTY(BlueprintReadOnly, Category = "Character", meta = (AllowPrivateAccess = true));
	float AO_Yaw;

	UPROPERTY(BlueprintReadOnly, Category = "Character", meta = (AllowPrivateAccess = true));
	float AO_Pitch;
	
	UPROPERTY(BlueprintReadOnly, Category = "Character", meta = (AllowPrivateAccess = true));
	FTransform LeftHandTransform;

	UPROPERTY(BlueprintReadOnly, Category = "Character", meta = (AllowPrivateAccess = true));
	ETurningInPlace TurningInPlace;

	UPROPERTY(BlueprintReadOnly, Category = "Character", meta = (AllowPrivateAccess = true));
	FRotator RightHandRotation;
	
	UPROPERTY(BlueprintReadOnly, Category = "Character", meta = (AllowPrivateAccess = true));
	bool bLocallyControlled;

	UPROPERTY(BlueprintReadOnly, Category = "Character", meta = (AllowPrivateAccess = true));
	bool bRotateRootBone;

	UPROPERTY(BlueprintReadOnly, Category = "Character", meta = (AllowPrivateAccess = true));
	bool bIsEliminated;

	UPROPERTY(BlueprintReadOnly, Category = "Character", meta = (AllowPrivateAccess = true));
	bool bUseFABRIK;

	UPROPERTY(BlueprintReadOnly, Category = "Character", meta = (AllowPrivateAccess = true));
	bool bUseAimOffset;

	UPROPERTY(BlueprintReadOnly, Category = "Character", meta = (AllowPrivateAccess = true));
	bool bTransformRightHand;

	UPROPERTY(BlueprintReadOnly, Category = "Character", meta = (AllowPrivateAccess = true));
	bool bHoldingTheFlag;

	UPROPERTY()
	AWeaponBase* EquippedWeapon;
	
	FRotator CharacterRotationLastFrame;
	FRotator CharacterRotation;
	FRotator DeltaRotation;
};
