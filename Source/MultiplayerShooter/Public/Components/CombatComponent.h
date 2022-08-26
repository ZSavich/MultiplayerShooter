// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "HUD/BlasterHUD.h"
#include "Weapons/WeaponBase.h"
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
	
public:	
	UCombatComponent();
	friend ABlasterCharacter;

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	void SetAiming(bool bIsAiming);
	void FireButtonPressed(bool bPressed);

	void SetHUDCrosshairs(float DeltaTime);
	
	UFUNCTION(Server, Reliable)
	void ServerSetAiming(bool bIsAiming);
	
protected:
	virtual void BeginPlay() override;

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
	
	UFUNCTION()
	void OnRep_EquippedWeapon();

private:
	UFUNCTION(Server, Reliable)
	void ServerFire(const FVector_NetQuantize& TraceHitTarget);

	UFUNCTION(NetMulticast, Reliable)
	void MulticastFire(const FVector_NetQuantize& TraceHitTarget);

	void InterpFOV(float DeltaTime);

	void StartFireTimer();
	void FireTimerFinished();
	void Fire();
};
