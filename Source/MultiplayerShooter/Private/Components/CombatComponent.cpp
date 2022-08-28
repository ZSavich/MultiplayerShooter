// Fill out your copyright notice in the Description page of Project Settings.

#include "Components/CombatComponent.h"

#include "Camera/CameraComponent.h"
#include "Character/BlasterCharacter.h"
#include "Engine/SkeletalMeshSocket.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"
#include "Weapons/WeaponBase.h"
#include "HUD/BlasterHUD.h"
#include "Character/BlasterPlayerController.h"
#include "Kismet/KismetMathLibrary.h"
#include "Sound/SoundCue.h"

UCombatComponent::UCombatComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	MaxWalkSpeed = 600.f;
	AimWalkSpeed = 350.f;
}

void UCombatComponent::BeginPlay()
{
	Super::BeginPlay();

	if (Character)
	{
		Character->GetCharacterMovement()->MaxWalkSpeed = MaxWalkSpeed;

		if (Character->GetFollowCamera())
		{
			DefaultFOV = Character->GetFollowCamera()->FieldOfView;
			CurrentFOV = DefaultFOV;
		}
		if (Character->HasAuthority())
		{
			InitializeCarriedAmmo();
		}
	}
}

void UCombatComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	
	if (Character && Character->IsLocallyControlled())
	{
		FHitResult HitResult;
		TraceUnderCrosshairs(HitResult);
		HitTarget = HitResult.ImpactPoint;
		InterpFOV(DeltaTime);
		SetHUDCrosshairs(DeltaTime);
	}
}

void UCombatComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UCombatComponent, EquippedWeapon);
	DOREPLIFETIME(UCombatComponent, bAiming);
	DOREPLIFETIME_CONDITION(UCombatComponent, CarriedAmmo, COND_OwnerOnly);
	DOREPLIFETIME(UCombatComponent, CombatState);
}

void UCombatComponent::SetAiming(bool bIsAiming)
{
	bAiming = bIsAiming;
	ServerSetAiming(bIsAiming);
	if (Character)
	{
		Character->GetCharacterMovement()->MaxWalkSpeed = bIsAiming ? AimWalkSpeed : MaxWalkSpeed;
	}
}

void UCombatComponent::ServerSetAiming_Implementation(bool bIsAiming)
{
	bAiming = bIsAiming;
	if (Character)
	{
		Character->GetCharacterMovement()->MaxWalkSpeed = bIsAiming ? AimWalkSpeed : MaxWalkSpeed;
	}
}

void UCombatComponent::EquipWeapon(AWeaponBase* WeaponToEquip)
{
	if (Character == nullptr || WeaponToEquip == nullptr)
	{
		return;
	}
	if (EquippedWeapon)
	{
		EquippedWeapon->Dropped();
	}
	
	EquippedWeapon = WeaponToEquip;
	
	EquippedWeapon->SetWeaponState(EWeaponState::EWS_Equipped);
	const USkeletalMeshSocket* HandSocket = Character->GetMesh()->GetSocketByName(TEXT("RightHandSocket"));
	if (HandSocket)
	{
		if (HandSocket->AttachActor(EquippedWeapon, Character->GetMesh()))
		{
			EquippedWeapon->ShowPickupWidget(false);
			EquippedWeapon->SetOwner(Character);
			EquippedWeapon->SetHUDAmmo();

			if (CarriedAmmoMap.Contains(EquippedWeapon->GetWeaponType()))
			{
				CarriedAmmo = CarriedAmmoMap[EquippedWeapon->GetWeaponType()];
			}

			Controller = Controller == nullptr ? Cast<ABlasterPlayerController>(Character->Controller) : Controller;
			if (Controller)
			{
				Controller->SetHUDCarriedAmmo(CarriedAmmo);
				Controller->SetHUDWeaponType(EquippedWeapon->GetWeaponType());
			}
			if (EquippedWeapon->EquipSoundFX)
			{
				UGameplayStatics::PlaySoundAtLocation(this, EquippedWeapon->EquipSoundFX, EquippedWeapon->GetActorLocation());
			}
			if (EquippedWeapon->IsEmpty())
			{
				Reload();
			}
			Character->GetCharacterMovement()->bOrientRotationToMovement = false;
			Character->bUseControllerRotationYaw = true;
		}
	}
}

void UCombatComponent::TraceUnderCrosshairs(FHitResult& HitResult)
{
	FVector2D ViewportSize;
	if (GEngine && GEngine->GameViewport)
	{
		GEngine->GameViewport->GetViewportSize(ViewportSize);
	}

	FVector2D CrosshairLocation = FVector2D(ViewportSize.X / 2.f, ViewportSize.Y / 2.f);

	FVector CrosshairWorldPosition;
	FVector CrosshairWorldDirection;
	const bool bScreenToWorld = UGameplayStatics::DeprojectScreenToWorld(
		UGameplayStatics::GetPlayerController(this, 0),
		CrosshairLocation,
		CrosshairWorldPosition,
		CrosshairWorldDirection
	);

	if (bScreenToWorld)
	{
		FVector Start = CrosshairWorldPosition;

		if (Character)
		{
			const float DistanceToCharacter = (Character->GetActorLocation() - Start).Size();
			Start += CrosshairWorldDirection * (DistanceToCharacter + 100.f);
		}
		
		const FVector End = Start + CrosshairWorldDirection * TRACE_LENGTH;

		if (UWorld* World = GetWorld())
		{
			World->LineTraceSingleByChannel(
				HitResult,
				Start,
				End,
				ECC_Visibility
			);
		}

		if (!HitResult.bBlockingHit)
		{
			HitResult.ImpactPoint = End;
		}

		if (HitResult.GetActor() && HitResult.GetActor()->Implements<UInteractWithCrosshairsInterface>())
		{
			HUDPackage.CrosshairColor = FLinearColor::Red;
		}
		else
		{
			HUDPackage.CrosshairColor = FLinearColor::White;
		}
	}
}

bool UCombatComponent::CanFire()
{
	if (EquippedWeapon == nullptr)
	{
		return false; 
	}
	return !EquippedWeapon->IsEmpty() && bCanFire && CombatState == ECombatState::ECS_Unoccupied;
}

void UCombatComponent::HandleReload()
{
	if (Character)
	{
		Character->PlayReloadMontage();
	}
}

int32 UCombatComponent::AmountToReload()
{
	if (EquippedWeapon)
	{
		const int32 RoomInMag = EquippedWeapon->GetMagCapacity() - EquippedWeapon->GetAmmo();
		if (CarriedAmmoMap.Contains(EquippedWeapon->GetWeaponType()))
		{
			const int32 AmmoCarried = CarriedAmmoMap[EquippedWeapon->GetWeaponType()];
			const int32 Least = FMath::Min(RoomInMag, AmmoCarried);
			return FMath::Clamp(RoomInMag, 0.f, Least);
		}
	}
	return 0;
}

void UCombatComponent::ServerReload_Implementation()
{
	if (Character == nullptr || EquippedWeapon == nullptr)
	{
		return;
	}

	CombatState = ECombatState::ECS_Reloading;
	HandleReload();
}

void UCombatComponent::OnRep_EquippedWeapon()
{
	if (EquippedWeapon == nullptr || Character == nullptr)
	{
		return;
	}

	EquippedWeapon->SetWeaponState(EWeaponState::EWS_Equipped);
	const USkeletalMeshSocket* HandSocket = Character->GetMesh()->GetSocketByName(TEXT("RightHandSocket"));
	if (HandSocket)
	{
		HandSocket->AttachActor(EquippedWeapon, Character->GetMesh());
	}
	if (EquippedWeapon->EquipSoundFX)
	{
		UGameplayStatics::PlaySoundAtLocation(this, EquippedWeapon->EquipSoundFX, EquippedWeapon->GetActorLocation());
	}
	Character->GetCharacterMovement()->bOrientRotationToMovement = false;
	Character->bUseControllerRotationYaw = true;
}

void UCombatComponent::OnRep_CarriedAmmo()
{
	if (Character == nullptr) return;
	
	Controller = Controller == nullptr ? Cast<ABlasterPlayerController>(Character->Controller) : Controller;
	if (Controller)
	{
		Controller->SetHUDCarriedAmmo(CarriedAmmo);
	}
}

void UCombatComponent::OnRep_CombatState()
{
	switch (CombatState)
	{
		case ECombatState::ECS_Reloading:
			HandleReload();
			break;
		case ECombatState::ECS_Unoccupied:
			if (bFireButtonPressed)
			{
				Fire();
			}
			break;
	}

}

void UCombatComponent::InterpFOV(float DeltaTime)
{
	if (EquippedWeapon == nullptr)
	{
		return;
	}

	if (bAiming)
	{
		CurrentFOV = FMath::FInterpTo(CurrentFOV, EquippedWeapon->GetZoomedFOV(), DeltaTime, EquippedWeapon->GetZoomInterpSpeed());
		
	}
	else
	{
		CurrentFOV = FMath::FInterpTo(CurrentFOV, DefaultFOV, DeltaTime, ZoomInterpSpeed);
	}
	if (Character && Character->GetFollowCamera())
	{
		Character->GetFollowCamera()->SetFieldOfView(CurrentFOV);
	}
}

void UCombatComponent::StartFireTimer()
{
	if (!EquippedWeapon || !Character)
	{
		return;
	}
	Character->GetWorldTimerManager().SetTimer(FireTimer, this, &UCombatComponent::FireTimerFinished, EquippedWeapon->FireDelay);
}

void UCombatComponent::FireTimerFinished()
{
	if (!EquippedWeapon || !Character)
	{
		return;
	}
	bCanFire = true;
	if (bFireButtonPressed && EquippedWeapon->bAutomatic)
	{
		Fire();
	}
	if (EquippedWeapon->IsEmpty())
	{
		Reload();
	}
}

void UCombatComponent::Fire()
{
	if (CanFire())
	{
		bCanFire = false;
		ServerFire(HitTarget);
		if (EquippedWeapon)
		{
			CrosshairShootingFactor = 0.75f;
		}
		StartFireTimer();
	}
	
}

void UCombatComponent::UpdateAmmoValues()
{
	if (Character == nullptr || EquippedWeapon == nullptr)
	{
		return;
	}
	const int32 ReloadAmount = AmountToReload();
	if (CarriedAmmoMap.Contains(EquippedWeapon->GetWeaponType()))
	{
		CarriedAmmoMap[EquippedWeapon->GetWeaponType()] -= ReloadAmount;
		CarriedAmmo = CarriedAmmoMap[EquippedWeapon->GetWeaponType()];
	}
	Controller = Controller == nullptr ? Cast<ABlasterPlayerController>(Character->Controller) : Controller;
	if (Controller)
	{
		Controller->SetHUDCarriedAmmo(CarriedAmmo);
	}
	EquippedWeapon->AddAmmo(-ReloadAmount);
}

void UCombatComponent::MulticastFire_Implementation(const FVector_NetQuantize& TraceHitTarget)
{
	if (EquippedWeapon && Character && CombatState == ECombatState::ECS_Unoccupied)
	{
		Character->PlayFireMontage(bAiming);
		EquippedWeapon->Fire(TraceHitTarget);
	}
}

void UCombatComponent::ServerFire_Implementation(const FVector_NetQuantize& TraceHitTarget)
{
	MulticastFire(TraceHitTarget);
}

void UCombatComponent::FireButtonPressed(bool bPressed)
{
	bFireButtonPressed = bPressed;
	if (bFireButtonPressed)
	{
		Fire();
	}
}

void UCombatComponent::Reload()
{
	if (CarriedAmmo > 0 && CombatState != ECombatState::ECS_Reloading)
	{
		ServerReload();
	}
}

void UCombatComponent::FinishReloading()
{
	if (Character && Character->HasAuthority())
	{
		CombatState = ECombatState::ECS_Unoccupied;
		UpdateAmmoValues();
	}
	if (bFireButtonPressed)
	{
		Fire();
	}
}

void UCombatComponent::SetHUDCrosshairs(float DeltaTime)
{
	if (!Character || !Character->Controller)
	{
		return;
	}

	Controller = Controller == nullptr ? Cast<ABlasterPlayerController>(Character->Controller) : Controller;

	if (Controller)
	{
		HUD = HUD == nullptr ? Cast<ABlasterHUD>(Controller->GetHUD()) : HUD;
		if (HUD)
		{
			if (EquippedWeapon)
			{
				if (EquippedWeapon->CrosshairCenter)
				{
					HUDPackage.CrosshairCenter = EquippedWeapon->CrosshairCenter;
				}
				if (EquippedWeapon->CrosshairLeft)
				{
					HUDPackage.CrosshairLeft = EquippedWeapon->CrosshairLeft;
				}
				if (EquippedWeapon->CrosshairRight)
				{
					HUDPackage.CrosshairRight = EquippedWeapon->CrosshairRight;
				}
				if (EquippedWeapon->CrosshairTop)
				{
					HUDPackage.CrosshairTop = EquippedWeapon->CrosshairTop;
				}
				if (EquippedWeapon->CrosshairBottom)
				{
					HUDPackage.CrosshairBottom = EquippedWeapon->CrosshairBottom;
				}
			}
			else
			{
				HUDPackage.CrosshairCenter = nullptr;
				HUDPackage.CrosshairLeft = nullptr;
				HUDPackage.CrosshairRight = nullptr;
				HUDPackage.CrosshairTop = nullptr;
				HUDPackage.CrosshairBottom = nullptr;
			}

			const FVector2D WalkSpeedRange(0.f, Character->GetCharacterMovement()->MaxWalkSpeed);
			const FVector2D VelocityMultiplayerRange(0.f, 1.f);
			FVector Velocity = Character->GetVelocity();
			Velocity.Z = 0.f;

			CrosshairVelocityFactor = FMath::GetMappedRangeValueClamped(
				WalkSpeedRange,
				VelocityMultiplayerRange,
				Velocity.Size()
			);

			if (Character->GetCharacterMovement()->IsFalling())
			{
				CrosshairInAirFactor = UKismetMathLibrary::FInterpTo(
					CrosshairInAirFactor,
					2.25f,
					DeltaTime,
					2.25f
				);
			}
			else
			{
				CrosshairInAirFactor = UKismetMathLibrary::FInterpTo(
					CrosshairInAirFactor,
					0.f,
					DeltaTime,
					30.f
				);
			}

			if (bAiming)
			{
				CrosshairAimFactor = FMath::FInterpTo(
					CrosshairAimFactor,
					0.58f,
					DeltaTime,
					30.f
				);
			}
			else
			{
				CrosshairAimFactor = FMath::FInterpTo(
					CrosshairShootingFactor,
					0.f,
					DeltaTime,
					30.f
				);
			}

			CrosshairShootingFactor = FMath::FInterpTo(
				CrosshairShootingFactor,
				0.f,
				DeltaTime,
				40.f
			);
			
			HUDPackage.CrosshairSpread = 0.5f + CrosshairInAirFactor + CrosshairVelocityFactor - CrosshairAimFactor + CrosshairShootingFactor;
			HUD->SetHUDPackage(HUDPackage);
		}
	}
}

void UCombatComponent::InitializeCarriedAmmo()
{
	CarriedAmmoMap.Emplace(EWeaponType::EWT_AssaultRifle, StartingARAmmo);
}