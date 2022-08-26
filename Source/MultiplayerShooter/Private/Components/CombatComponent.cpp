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
	return !EquippedWeapon->IsEmpty() || !bCanFire;
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
	Character->GetCharacterMovement()->bOrientRotationToMovement = false;
	Character->bUseControllerRotationYaw = true;
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

void UCombatComponent::MulticastFire_Implementation(const FVector_NetQuantize& TraceHitTarget)
{
	if (EquippedWeapon && Character)
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
