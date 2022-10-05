// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BlasterTypes/CombatState.h"
#include "Components/ActorComponent.h"
#include "HUD/BlasterHUD.h"
#include "Projectiles/ProjectileBase.h"
#include "Weapons/WeaponBase.h"
#include "Weapons/WeaponTypes.h"
#include "CombatComponent.generated.h"

class ABlasterCharacter;
class AWeaponBase;
class ABlasterPlayerController;
class ABlasterHUD;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class MULTIPLAYERSHOOTER_API UCombatComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	bool bLocallyReloading = false;
	
private:
	UPROPERTY()
	ABlasterPlayerController* Controller;

	UPROPERTY()
	ABlasterHUD* HUD;
	
	UPROPERTY(ReplicatedUsing = OnRep_HoldingTheFlag)
	bool bHoldingTheFlag = false;

	float CrosshairVelocityFactor;
	float CrosshairInAirFactor;
	float CrosshairAimFactor;
	float CrosshairShootingFactor;

	FVector HitTarget;

	UPROPERTY(EditAnywhere, Category = "FOV")
	float ZoomedFOV = 30.f;

	UPROPERTY(EditAnywhere, Category = "FOV")
	float ZoomInterpSpeed = 20.f;
	
	float CurrentFOV;
	float DefaultFOV;

	FHUDPackage HUDPackage;

	FTimerHandle FireTimer;
	bool bCanFire = true;

	/*
	 * Ammo data
	 */
	UPROPERTY(ReplicatedUsing = OnRep_CarriedAmmo)
	float CarriedAmmo;

	UPROPERTY(EditAnywhere, Category = "Starting Ammo")
	float StartingARAmmo = 30.f;

	UPROPERTY(EditAnywhere, Category = "Starting Ammo")
	float StartingRocketAmmo = 0.f;

	UPROPERTY(EditAnywhere, Category = "Starting Ammo")
	float StartingPistolAmmo = 0.f;

	UPROPERTY(EditAnywhere, Category = "Starting Ammo")
	float StartingSubmachineGunAmmo = 0.f;

	UPROPERTY(EditAnywhere, Category = "Starting Ammo")
	float StartingShotgunAmmo = 0.f;
	
	UPROPERTY(EditAnywhere, Category = "Starting Ammo")
	float StartingSniperRifleAmmo = 0.f;

	UPROPERTY(EditAnywhere, Category = "Starting Ammo")
	float StartingGrenadeLauncherAmmo = 0.f;

	UPROPERTY(EditAnywhere, Category = "Ammount")
	float MaxCarriedAmmo = 500;
	
	TMap<EWeaponType, uint32> CarriedAmmoMap;

	UPROPERTY(ReplicatedUsing = OnRep_Grenades)
	int32 Grenades = 4;

	UPROPERTY(EditAnywhere, Category = "Grenades")
	int32 MaxGrenades = 4;

	/*
	 * Reloading
	 */
	UPROPERTY(ReplicatedUsing = OnRep_CombatState)
	ECombatState CombatState = ECombatState::ECS_Unoccupied;

protected:
	UPROPERTY(EditAnywhere, Category = "Grenade")
	TSubclassOf<AProjectileBase> GrenadeClass;
	
public:	
	UCombatComponent();
	friend ABlasterCharacter;
	
	UFUNCTION(Server, Reliable)
	void ServerSetAiming(bool bIsAiming);
	
	UFUNCTION(BlueprintCallable)
	void FinishReloading();

	UFUNCTION(BlueprintCallable)
	void ShotgunShellReload();

	UFUNCTION(BlueprintCallable)
	void ThrowGrenadeFinished();

	UFUNCTION(BlueprintCallable)
	void LaunchGrenade();
	
	UFUNCTION(BlueprintCallable)
	void FinishSwap();
	
	UFUNCTION(BlueprintCallable)
	void FinishSwapAttachWeapons();
	
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	void SetAiming(bool bIsAiming);
	void FireButtonPressed(bool bPressed);
	void Reload();
	
	void SetHUDCrosshairs(float DeltaTime);

	void JumpToShotgunEnd();

	void UpdateHUDGrenades();

	void PickupAmmo(EWeaponType WeaponType, int32 AmmoAmount);

	void SwapWeapon();
	bool ShouldSwapWeapon();
	
	FORCEINLINE int32 GetGrenades() const { return Grenades; }
	
protected:
	virtual void BeginPlay() override;
	void InitializeCarriedAmmo();

	UPROPERTY(ReplicatedUsing = OnRep_EquippedWeapon)
	AWeaponBase* EquippedWeapon;

	UPROPERTY(ReplicatedUsing = OnRep_SecondaryWeapon)
	AWeaponBase* SecondaryWeapon;

	UPROPERTY()
	ABlasterCharacter* Character;

	UPROPERTY(ReplicatedUsing = OnRep_Aiming)
	bool bAiming = false;

	UPROPERTY(EditDefaultsOnly, Category = "Movement")
	float MaxWalkSpeed;

	UPROPERTY(EditDefaultsOnly, Category = "Movement")
	float AimWalkSpeed;

	bool bFireButtonPressed = false;
	bool bAimButtonPressed = false;

	void EquipWeapon(AWeaponBase* WeaponToEquip);

	void EquipPrimaryWeapon(AWeaponBase* WeaponToEquip);
	void EquipSecondaryWeapon(AWeaponBase* WeaponToEquip);
	
	void TraceUnderCrosshairs(FHitResult& HitResult);
	bool CanFire();

	void DropEquippedWeapon();
	bool AttachActorToRightHand(AActor* ActorToAttach);
	bool AttachActorToLeftHand(AActor* ActorToAttach);
	bool AttachActorToBackpack(AActor* ActorToAttach);
	bool AttachFlagToLeftHand(AWeaponBase* FlagToAttach);
	
	void UpdateCarriedAmmo();
	void PlayEquipWeaponSound(AWeaponBase* WeaponToEquip);
	void ReloadEmptyWeapon();

	/*
	 * Reloading
	 */
	void HandleReload();
	int32 AmountToReload();

	/*
	 * Throw Grenade
	 */
	void ThrowGrenade();
	UFUNCTION(Server, Reliable)
	void ServerThrowGrenade();

	void ShowAttachedGrenade(const bool bShowGrenade);

	/*
	 * Multiplayer
	 */
	UFUNCTION(Server, Reliable)
	void ServerReload();
	
	UFUNCTION()
	void OnRep_EquippedWeapon();

	UFUNCTION()
	void OnRep_SecondaryWeapon();

	UFUNCTION()
	void OnRep_CarriedAmmo();

	UFUNCTION()
	void OnRep_CombatState();

	UFUNCTION()
	void OnRep_Grenades();

	UFUNCTION()
	void OnRep_Aiming();

	UFUNCTION()
	void OnRep_HoldingTheFlag();

private:
	UFUNCTION(Server, Reliable, WithValidation)
	void ServerFire(const FVector_NetQuantize& TraceHitTarget, float FireDelay);

	UFUNCTION(NetMulticast, Reliable)
	void MulticastFire(const FVector_NetQuantize& TraceHitTarget);

	UFUNCTION(Server, Reliable)
	void ServerLaunchGrenade(const FVector_NetQuantize& Target);

	UFUNCTION(Server, Reliable, WithValidation)
	void ServerShotgunFire(const TArray<FVector_NetQuantize>& TraceHitTargets, float FireDelay);

	UFUNCTION(NetMulticast, Reliable)
	void MulticastShotgunFire(const TArray<FVector_NetQuantize>& TraceHitTargets);

	void InterpFOV(float DeltaTime);

	void StartFireTimer();
	void FireTimerFinished();
	void Fire();
	void LocalFire(const FVector_NetQuantize& TraceHitTarget);
	void ShotgunLocalFire(const TArray<FVector_NetQuantize>& TraceHitTargets);

	void FireProjectileWeapon();
	void FireHitScanWeapon();
	void FireShotgun();

	void UpdateAmmoValues();

	void UpdateShotgunAmmoValues();
};
