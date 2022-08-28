// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CasingBase.h"
#include "WeaponTypes.h"
#include "GameFramework/Actor.h"
#include "WeaponBase.generated.h"

class USphereComponent;
class UWidgetComponent;
class ABlasterCharacter;
class ABlasterPlayerController;

UENUM()
enum class EWeaponState : uint8
{
	EWS_Initial		UMETA(DisplayName = "Initial State"),
	EWS_Equipped	UMETA(DisplayName = "Equipped State"),
	EWS_Dropped		UMETA(DisplayName = "Dropped State"),

	EWS_MAX			UMETA(Hidden)
};

UCLASS()
class MULTIPLAYERSHOOTER_API AWeaponBase : public AActor
{
	GENERATED_BODY()

private:
	UPROPERTY(EditAnywhere, Category = "FOV")
	float ZoomedFOV = 45.f;

	UPROPERTY(EditAnywhere, Category = "FOV")
	float ZoomInterpSpeed = 30.f;

	UPROPERTY(EditAnywhere, ReplicatedUsing = OnRep_Ammo, Category = "Ammo")
	int32 Ammo = 30.f;

	UPROPERTY(EditAnywhere, Category = "Ammo")
	int32 MagCapacity = 30.f;

	UPROPERTY(EditAnywhere, Category = "Weapom")
	EWeaponType WeaponType;

	UPROPERTY()
	ABlasterCharacter* BlasterOwnerCharacter;

	UPROPERTY()
	ABlasterPlayerController* BlasterOwnerController; 
	
	UFUNCTION()
	void OnRep_Ammo();
	void SpendRound();
	
public:	
	AWeaponBase();

	UPROPERTY(EditDefaultsOnly, Category = "Crosshairs")
	UTexture2D* CrosshairCenter;
	
	UPROPERTY(EditDefaultsOnly, Category = "Crosshairs")
	UTexture2D* CrosshairLeft;

	UPROPERTY(EditDefaultsOnly, Category = "Crosshairs")
	UTexture2D* CrosshairRight;

	UPROPERTY(EditDefaultsOnly, Category = "Crosshairs")
	UTexture2D* CrosshairTop;

	UPROPERTY(EditDefaultsOnly, Category = "Crosshairs")
	UTexture2D* CrosshairBottom;

	UPROPERTY(EditAnywhere, Category = "Weapon")
	bool bAutomatic = true;

	UPROPERTY(EditAnywhere, Category = "Weapon")
	float FireDelay = 0.15f;

	UPROPERTY(EditDefaultsOnly, Category = "Sounds")
	USoundCue* EquipSoundFX;
	
protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon")
	USkeletalMeshComponent* WeaponMeshComponent;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collision")
	USphereComponent* AreaSphere;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon")
	UWidgetComponent* PickupWidget;

	UPROPERTY(ReplicatedUsing = OnRep_WeaponState, VisibleAnywhere, Category = "Weapon")
	EWeaponState WeaponState;

	UPROPERTY(EditAnywhere, Category = "Animations")
	UAnimationAsset* FireAnimation;

	UPROPERTY(EditAnywhere, Category = "Weapon")
	TSubclassOf<ACasingBase> CasingClass;
	
	virtual void BeginPlay() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	
	UFUNCTION()
	void OnSphereOverlapped(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult & SweepResult);

	UFUNCTION()
	void OnSphereEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	UFUNCTION()
	void OnRep_WeaponState();

public:
	virtual void OnRep_Owner() override;
	void ShowPickupWidget(bool IsVisible) const;
	virtual void Fire(const FVector& HitTarget);

	void SetHUDAmmo();
	bool IsEmpty();
	void Dropped();
	void AddAmmo(const int32 AmmoToAdd);

	FORCEINLINE void SetWeaponState(EWeaponState InWeaponState);
	FORCEINLINE USkeletalMeshComponent* GetWeaponMesh() const { return WeaponMeshComponent; }
	FORCEINLINE float GetZoomedFOV() const { return ZoomedFOV; }
	FORCEINLINE float GetZoomInterpSpeed() const { return ZoomInterpSpeed; }
	FORCEINLINE const EWeaponType&  GetWeaponType() const { return WeaponType; }
	FORCEINLINE int32 GetMagCapacity() const { return MagCapacity; }
	FORCEINLINE int32 GetAmmo() const { return Ammo; }
};
