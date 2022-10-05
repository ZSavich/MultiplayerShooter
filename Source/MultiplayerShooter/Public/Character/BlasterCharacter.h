// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BlasterTypes/TurningInPlace.h"
#include "Components/CombatComponent.h"
#include "Components/TimelineComponent.h"
#include "GameFramework/Character.h"
#include "GameModes/BlasterGameMode.h"
#include "Interfaces/InteractWithCrosshairsInterface.h"
#include "PlayerStates/BlasterPlayerState.h"
#include "BlasterCharacter.generated.h"

class ULagCompensationComponent;
class UBuffComponent;
class UCameraComponent;
class USpringArmComponent;
class AWeaponBase;
class UWidgetComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnLeftGame);

UCLASS()
class MULTIPLAYERSHOOTER_API ABlasterCharacter : public ACharacter, public IInteractWithCrosshairsInterface
{
	GENERATED_BODY()

private:
	UPROPERTY(VisibleAnywhere, Category = "Camera")
	USpringArmComponent* CameraBoom;

	UPROPERTY(VisibleAnywhere, Category = "Camera")
	UCameraComponent* FollowCamera;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = true))
	UCombatComponent* CombatComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = true))
	UBuffComponent* BuffComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = true))
	ULagCompensationComponent* LagCompensationComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Widgets", meta = (AllowPrivateAccess = true))
	UWidgetComponent* OverheadWidget;

	UPROPERTY(VisibleAnywhere)
	UStaticMeshComponent* AttachedGrenadeMesh;

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

	UPROPERTY(EditAnywhere, Category = "Animations")
	UAnimMontage* ThrowGrenadeMontage;

	UPROPERTY(EditAnywhere, Category = "Animations")
	UAnimMontage* SwapMontage;

	UPROPERTY()
	ABlasterPlayerController* BlasterPlayerController;

	UPROPERTY()
	ABlasterGameMode* BlasterGameMode;

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

	bool bLeftGame = false;

	/*
	 * Health Properties
	 */
	UPROPERTY(EditDefaultsOnly, Category = "Player Stats")
	float MaxHealth = 100.f;

	UPROPERTY(ReplicatedUsing = OnRep_Health, EditDefaultsOnly, Category = "Player Stats")
	float Health = 100.f;

	/*
	 * Shield Properties
	 */
	UPROPERTY(EditDefaultsOnly, Category = "Player Stats")
	float MaxShield = 100.f;

	UPROPERTY(ReplicatedUsing = OnRep_Shield, EditDefaultsOnly, Category = "Player State")
	float Shield = 0.f;
	
	/*
	 * Elimination
	 */
	FTimerHandle EliminateTimerHandle;
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

	/*
	 * Team Materials
	 */
	UPROPERTY(EditAnywhere, Category = "Team")
	UMaterialInstance* RedDissolvedMatInst;

	UPROPERTY(EditAnywhere, Category = "Team")
	UMaterialInstance* BlueDissolvedMatInst;

	UPROPERTY(EditAnywhere, Category = "Team")
	UMaterialInstance* RedTeamMaterial;

	UPROPERTY(EditAnywhere, Category = "Team")
	UMaterialInstance* BlueTeamMaterial;

	UPROPERTY(EditAnywhere, Category = "Team")
	UMaterialInstance* DefaultTeamMaterial;

	/* Default Weapon */
	UPROPERTY(EditAnywhere)
	TSubclassOf<AWeaponBase> DefaultWeaponClass;

	/* Lag Compensation Boxes */
	UPROPERTY(EditAnywhere)
	UBoxComponent* head;

	UPROPERTY(EditAnywhere)
	UBoxComponent* pelvis;

	UPROPERTY(EditAnywhere)
	UBoxComponent* spine_02;

	UPROPERTY(EditAnywhere)
	UBoxComponent* spine_03;

	UPROPERTY(EditAnywhere)
	UBoxComponent* upperarm_l;

	UPROPERTY(EditAnywhere)
	UBoxComponent* upperarm_r;

	UPROPERTY(EditAnywhere)
	UBoxComponent* lowerarm_l;

	UPROPERTY(EditAnywhere)
	UBoxComponent* lowerarm_r;

	UPROPERTY(EditAnywhere)
	UBoxComponent* hand_l;

	UPROPERTY(EditAnywhere)
	UBoxComponent* hand_r;

	UPROPERTY(EditAnywhere)
	UBoxComponent* backpack;

	UPROPERTY(EditAnywhere)
	UBoxComponent* blanket;

	UPROPERTY(EditAnywhere)
	UBoxComponent* thigh_l;

	UPROPERTY(EditAnywhere)
	UBoxComponent* thigh_r;

	UPROPERTY(EditAnywhere)
	UBoxComponent* calf_l;

	UPROPERTY(EditAnywhere)
	UBoxComponent* calf_r;

	UPROPERTY(EditAnywhere)
	UBoxComponent* foot_l;

	UPROPERTY(EditAnywhere)
	UBoxComponent* foot_r;

protected:
	/*
	 * Eliminate Bot 
	 */
	UPROPERTY(EditAnywhere, Category = "Eliminate Bot")
	UParticleSystem* ElimBotParticleFX;

	UPROPERTY(VisibleAnywhere, Category = "Eliminate Bot")
	UParticleSystemComponent* ElimBotComponent;

	UPROPERTY(EditAnywhere, Category = "Eliminate Bot")
	USoundCue* ElimBotSoundFX;

	UPROPERTY(EditAnywhere, Category = "Niagara")
	UNiagaraSystem* CrownNiagaraSystem;

	UPROPERTY()
	UNiagaraComponent* CrownNiagaraComponent;
	
public:
	UPROPERTY()
	ABlasterPlayerState* BlasterPlayerState;

	UPROPERTY(Replicated)
	bool bDisableGameplay = false;

	UPROPERTY()
	TMap<FName, UBoxComponent*> HitCollisionBoxes;

	bool bFinishedSwapping = false;
	
	FOnLeftGame OnLeftGame;
	
private:
	UFUNCTION()
	void UpdateDissolveMaterial(float DissolveValue);
	void StartDissolve();
	
	UFUNCTION()
	void OnRep_OverlappingWeapon(const AWeaponBase* LastWeapon);

	UFUNCTION()
	void OnRep_Health(float LastHealth);

	UFUNCTION()
	void OnRep_Shield(float LastShield);

	void HideCameraIfCharacterClose();
	void PlayHitReactMontage();
	float CalculateSpeed() const;
	void CalculateAO_Pitch();
	
	void EliminateTimerFinished();
	
protected:
	UFUNCTION(Server, Reliable)
	void ServerEquipButtonPressed();

	UFUNCTION()
	void ReceivedDamage(AActor* DamagedActor, float Damage, const class UDamageType* DamageType, class AController* InstigatedBy, AActor* DamageCauser );
	
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
	void GrenadeButtonPressed();

	void FireButtonPressed();
	void FireButtonReleased();

	void ReloadButtonPressed();
	
	void TurnInPlace(float DeltaTime);

	void SimProxiesTurn();
	
	void PollInit();

	void RotateInPlace(const float DeltaTime);

	void DropOrDestroyWeapon(AWeaponBase* Weapon);
	void DropOrDestroyWeapons();

	void SetSpawnPoint();
	void OnPlayerStateInitialized();
	
public:
	ABlasterCharacter();

	UFUNCTION(BlueprintImplementableEvent)
	void ShowSniperScopeWidget(const bool bShowScope);
	
	UFUNCTION(NetMulticast, Reliable)
	void MulticastEliminate(bool bPlayerLeftGame);

	UFUNCTION(Server, Reliable)
	void ServerLeaveGame();

	UFUNCTION(NetMulticast, Reliable)
	void MulticastGainedTheLead();

	UFUNCTION(NetMulticast, Reliable)
	void MulticastLostTheLead();
	
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void PostInitializeComponents() override;
	virtual void Tick(float DeltaSeconds) override;
	virtual void OnRep_ReplicatedMovement() override;
	virtual void Destroyed() override;
	
	void SetOverlappingWeapon(AWeaponBase* Weapon);
	bool IsWeaponEquipped() const;

	void UpdateHUDHealth();
	void UpdateHUDShield();
	void UpdateHUDAmmo();

	void SpawnDefaultWeapon();
	
	bool GetIsAiming() const;
	AWeaponBase* GetEquippedWeapon() const;

	FVector GetHitTarget() const;

	void PlayFireMontage(bool bAiming);
	void PlayEliminateMontage();
	void PlayReloadMontage();
	void PlayThrowGrenadeMontage();
	void PlaySwapMontage();
	
	void Eliminate(bool bPlayerLeftGame);

	ECombatState GetCombatState() const;

	bool IsLocallyReloading();

	void SetTeamColor(const ETeam Team);

	bool IsHoldingTheFlag() const;

	ETeam GetTeam();

	void SetHoldingTheFlag(bool bHolding);
	
	FORCEINLINE float GetAO_Yaw() const { return AO_Yaw; }
	FORCEINLINE float GetAO_Pitch() const { return AO_Pitch; }
	FORCEINLINE ETurningInPlace GetTurningInPlace() const { return TurningInPlace; }
	FORCEINLINE UCameraComponent* GetFollowCamera() const { return FollowCamera; }
	FORCEINLINE bool ShouldRotateRootBone() const { return bRotateRootBone; }
	FORCEINLINE bool IsEliminated() const { return bIsEliminated; }
	FORCEINLINE float GetHealth() const { return Health; }
	FORCEINLINE void SetHealth(float Amount) { Health = Amount; }
	FORCEINLINE float GetMaxHealth() const { return MaxHealth; }
	FORCEINLINE float GetShield() const { return Shield; }
	FORCEINLINE void SetShield(float Amount) { Shield = Amount; }
	FORCEINLINE float GetMaxShield() const { return MaxShield; }
	FORCEINLINE UCombatComponent* GetCombatComponent() const { return CombatComponent; }
	FORCEINLINE UBuffComponent* GetBuffComponent() const { return BuffComponent; }
	FORCEINLINE ULagCompensationComponent* GetLagCompensationComponent() const { return LagCompensationComponent; }
	FORCEINLINE bool GetDisableGameplay() const { return bDisableGameplay; }
	FORCEINLINE UAnimMontage* GetReloadMontage() const { return ReloadMontage; }
	FORCEINLINE UStaticMeshComponent* GetAttachedGrenadeMesh() const { return AttachedGrenadeMesh; }
};
