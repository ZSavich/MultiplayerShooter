// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BlasterTypes/TurningInPlace.h"
#include "Components/CombatComponent.h"
#include "Components/TimelineComponent.h"
#include "GameFramework/Character.h"
#include "Interfaces/InteractWithCrosshairsInterface.h"
#include "PlayerStates/BlasterPlayerState.h"
#include "BlasterCharacter.generated.h"

class UCameraComponent;
class USpringArmComponent;
class AWeaponBase;
class UWidgetComponent;

UCLASS()
class MULTIPLAYERSHOOTER_API ABlasterCharacter : public ACharacter, public IInteractWithCrosshairsInterface
{
	GENERATED_BODY()
	
public:
	UPROPERTY()
	ABlasterPlayerState* BlasterPlayerState;
	
public:
	ABlasterCharacter();
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void PostInitializeComponents() override;
	virtual void Tick(float DeltaSeconds) override;
	virtual void OnRep_ReplicatedMovement() override;
	virtual void Destroyed() override;
	
	void SetOverlappingWeapon(AWeaponBase* Weapon);
	bool IsWeaponEquipped() const;

	bool GetIsAiming() const;
	AWeaponBase* GetEquippedWeapon() const;

	FVector GetHitTarget() const;

	void PlayFireMontage(bool bAiming);
	void PlayEliminateMontage();
	void PlayReloadMontage();
	
	void Eliminate();
	UFUNCTION(NetMulticast, Reliable)
	void MulticastEliminate();

	ECombatState GetCombatState() const;
	
	FORCEINLINE float GetAO_Yaw() const { return AO_Yaw; }
	FORCEINLINE float GetAO_Pitch() const { return AO_Pitch; }
	FORCEINLINE ETurningInPlace GetTurningInPlace() const { return TurningInPlace; }
	FORCEINLINE UCameraComponent* GetFollowCamera() const { return FollowCamera; }
	FORCEINLINE bool ShouldRotateRootBone() const { return bRotateRootBone; }
	FORCEINLINE bool IsEliminated() const { return bIsEliminated; }
	FORCEINLINE float GetHealth() const { return Health; }
	FORCEINLINE float GetMaxHealth() const { return MaxHealth; }
	
protected:
	virtual void BeginPlay() override;
	virtual void Jump() override;
	void MoveForward(const float Value);
	void MoveRight(const float Value);
	void LookUp(const float Value);
	void Turn(const float Value);
	void EquipButtonPressed();
	void CrouchButtonPressed();
	void AimButtonPressed();
	void AimButtonReleased();
	void AimOffset(float DeltaTime);

	void FireButtonPressed();
	void FireButtonReleased();

	void ReloadButtonPressed();
	
	void TurnInPlace(float DeltaTime);

	void SimProxiesTurn();
	
	void PollInit();

	/*
	 * Eliminate Bot 
	 */
	UPROPERTY(EditAnywhere, Category = "Eliminate Bot")
	UParticleSystem* ElimBotParticleFX;

	UPROPERTY(VisibleAnywhere, Category = "Eliminate Bot")
	UParticleSystemComponent* ElimBotComponent;

	UPROPERTY(EditAnywhere, Category = "Eliminate Bot")
	USoundCue* ElimBotSoundFX;
	
	
	UFUNCTION(Server, Reliable)
	void ServerEquipButtonPressed();

	UFUNCTION()
	void ReceivedDamage(AActor* DamagedActor, float Damage, const class UDamageType* DamageType, class AController* InstigatedBy, AActor* DamageCauser );
	void UpdateHUDHealth();

private:
	UPROPERTY(VisibleAnywhere, Category = "Camera")
	USpringArmComponent* CameraBoom;

	UPROPERTY(VisibleAnywhere, Category = "Camera")
	UCameraComponent* FollowCamera;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = true))
	UCombatComponent* CombatComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Widgets", meta = (AllowPrivateAccess = true))
	UWidgetComponent* OverheadWidget;

	UPROPERTY(ReplicatedUsing = OnRep_OverlappingWeapon)
	AWeaponBase* OverlappingWeapon;

	UPROPERTY(EditAnywhere, Category = "Animations")
	UAnimMontage* FireWeaponMontage;

	UPROPERTY(EditAnywhere, Category = "Animations")
	UAnimMontage* HitReactMontage;

	UPROPERTY(EditAnywhere, Category = "Animations")
	UAnimMontage* EliminateMontage;

	UPROPERTY(EditAnywhere, Category = "Animations")
	UAnimMontage* ReloadMontage;

	UPROPERTY()
	ABlasterPlayerController* BlasterPlayerController;

	float AO_Yaw;
	float AO_Pitch;
	float InterpAO_Yaw;
	FRotator StartingAimRotation;
	
	bool bRotateRootBone;
	FRotator ProxyRotationLastFrame;
	FRotator ProxyRotation;
	float ProxyYaw;
	float TurnThreshold = 0.5;

	float TimeSinceLastMovementReplication;

	ETurningInPlace TurningInPlace;

	float CameraThreshold = 200.f;

	bool bIsEliminated = false;

	/*
	 * Health Properties
	 */
	UPROPERTY(EditDefaultsOnly, Category = "Player Stats")
	float MaxHealth = 100.f;

	UPROPERTY(ReplicatedUsing = OnRep_Health, EditDefaultsOnly, Category = "Player Stats")
	float Health = 100.f;

	/*
	 * Elimination
	 */
	FTimerHandle EliminateTimerHandle;
	void EliminateTimerFinished();
	float EliminateDelay = 3.f;

	/*
	 * Dissolve effect
	 */
	FOnTimelineFloat DissolveTrack;

	UPROPERTY(VisibleAnywhere, Category = "Components")
	UTimelineComponent* DissolveTimeline;

	UPROPERTY(EditAnywhere, Category = "Dissolve effect")
	UCurveFloat* DissolveCurve;

	UPROPERTY(VisibleAnywhere, Category = "Dissolve effect")
	UMaterialInstanceDynamic* DynamicDissolveMaterialInstance;

	UPROPERTY(EditAnywhere, Category = "Dissolve effect")
	UMaterialInstance* DissolveMaterialInstance;

	UFUNCTION()
	void UpdateDissolveMaterial(const float DissolveValue);
	void StartDissolve();
	
	UFUNCTION()
	void OnRep_OverlappingWeapon(const AWeaponBase* LastWeapon);

	UFUNCTION()
	void OnRep_Health();

	void HideCameraIfCharacterClose();
	void PlayHitReactMontage();
	float CalculateSpeed() const;
	void CalculateAO_Pitch();
};
