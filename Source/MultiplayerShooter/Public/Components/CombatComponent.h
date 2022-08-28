// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BlasterTypes/CombatState.h"
#include "Components/ActorComponent.h"
#include "HUD/BlasterHUD.h"
#include "Weapons/WeaponBase.h"
#include "Weapons/WeaponTypes.h"
#include "CombatComponent.generated.h"

class ABlasterCharacter;
class AWeaponBase;
class ABlasterPlayerController;
class ABlasterHUD;

#define TRACE_LENGTH 80000.f;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class MULTIPLAYERSHOOTER_API UCombatComponent : public UActorComponent
{
	GENERATED_BODY()

private:
	UPROPERTY()
	ABlasterPlayerController* Controller;

	UPROPERTY()
	ABlasterHUD* HUD;

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

	UPROPERTY(EditAnywhere, Category = "Ammo")
	float StartingARAmmo = 30.f;
	
	TMap<EWeaponType, uint32> CarriedAmmoMap;

	/*
	 * Reloading
	 */
	UPROPERTY(ReplicatedUsing = OnRep_CombatState)
	ECombatState CombatState = ECombatState::ECS_Unoccupied;
	
public:	
	UCombatComponent();
	friend ABlasterCharacter;
	
	UFUNCTION(Server, Reliable)
	void ServerSetAiming(bool bIsAiming);
	
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	void SetAiming(bool bIsAiming);
	void FireButtonPressed(bool bPressed);
	void Reload();
	
	UFUNCTION(BlueprintCallable)
	void FinishReloading();
	
	void SetHUDCrosshairs(float DeltaTime);
	
protected:
	virtual void BeginPlay() override;
	void InitializeCarriedAmmo();

	UPROPERTY(ReplicatedUsing = OnRep_EquippedWeapon)
	AWeaponBase* EquippedWeapon;

	UPROPERTY()
	ABlasterCharacter* Character;

	UPROPERTY(Replicated)
	bool bAiming;

	UPROPERTY(EditDefaultsOnly, Category = "Movement")
	float MaxWalkSpeed;

	UPROPERTY(EditDefaultsOnly, Category = "Movement")
	float AimWalkSpeed;

	bool bFireButtonPressed;

	void EquipWeapon(AWeaponBase* WeaponToEquip);
	void TraceUnderCrosshairs(FHitResult& HitResult);
	bool CanFire();

	/*
	 * Reloading
	 */
	void HandleReload();
	int32 AmountToReload();

	/*
	 * Multiplayer
	 */
	UFUNCTION(Server, Reliable)
	void ServerReload();
	
	UFUNCTION()
	void OnRep_EquippedWeapon();

	UFUNCTION()
	void OnRep_CarriedAmmo();

	UFUNCTION()
	void OnRep_CombatState();

private:
	UFUNCTION(Server, Reliable)
	void ServerFire(const FVector_NetQuantize& TraceHitTarget);

	UFUNCTION(NetMulticast, Reliable)
	void MulticastFire(const FVector_NetQuantize& TraceHitTarget);

	void InterpFOV(float DeltaTime);

	void StartFireTimer();
	void FireTimerFinished();
	void Fire();

	void UpdateAmmoValues();
};
