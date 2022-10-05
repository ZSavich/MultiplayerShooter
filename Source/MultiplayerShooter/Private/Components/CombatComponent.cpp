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
#include "Weapons/ShotgunWeapon.h"
#include "Weapons/WeaponTypes.h"

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
	DOREPLIFETIME(UCombatComponent, SecondaryWeapon);
	DOREPLIFETIME(UCombatComponent, bAiming);
	DOREPLIFETIME_CONDITION(UCombatComponent, CarriedAmmo, COND_OwnerOnly);
	DOREPLIFETIME(UCombatComponent, CombatState);
	DOREPLIFETIME(UCombatComponent, Grenades);
	DOREPLIFETIME(UCombatComponent, bHoldingTheFlag);
}

void UCombatComponent::SetAiming(bool bIsAiming)
{
	if (Character == nullptr || Character->GetEquippedWeapon() == nullptr)
	{
		return;
	}
	bAiming = bIsAiming;
	ServerSetAiming(bIsAiming);
	if (Character)
	{
		Character->GetCharacterMovement()->MaxWalkSpeed = bIsAiming ? AimWalkSpeed : MaxWalkSpeed;
	}
	const bool bIsSniperRifle = Character && Character->IsLocallyControlled() && Character->GetEquippedWeapon() && Character->GetEquippedWeapon()->GetWeaponType() == EWeaponType::EWT_SniperRifle;
	if (bIsSniperRifle)
	{
		Character->ShowSniperScopeWidget(bIsAiming);
	}
	if (Character->IsLocallyControlled())
	{
		bAimButtonPressed = bIsAiming;
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
	if (CombatState != ECombatState::ECS_Unoccupied)
	{
		return;
	}
	if (WeaponToEquip->GetWeaponType() == EWeaponType::EWT_Flag)
	{
		Character->Crouch();
		bHoldingTheFlag = true;
		WeaponToEquip->SetWeaponState(EWeaponState::EWS_Equipped);
		AttachFlagToLeftHand(WeaponToEquip);
	}
	else
	{
		if (EquippedWeapon && SecondaryWeapon == nullptr)
		{
			EquipSecondaryWeapon(WeaponToEquip);
		}
		else
		{
			EquipPrimaryWeapon(WeaponToEquip);
		}
	
		Character->GetCharacterMovement()->bOrientRotationToMovement = false;
		Character->bUseControllerRotationYaw = true;
	}
}

void UCombatComponent::EquipPrimaryWeapon(AWeaponBase* WeaponToEquip)
{
	if (WeaponToEquip == nullptr) return;
	DropEquippedWeapon();
	EquippedWeapon = WeaponToEquip;
	EquippedWeapon->SetWeaponState(EWeaponState::EWS_Equipped);
	EquippedWeapon->SetOwner(Character);
	EquippedWeapon->SetHUDAmmo();
	if (AttachActorToRightHand(EquippedWeapon))
	{
		UpdateCarriedAmmo();
		PlayEquipWeaponSound(WeaponToEquip);
		ReloadEmptyWeapon();
	}
}

void UCombatComponent::EquipSecondaryWeapon(AWeaponBase* WeaponToEquip)
{
	if (WeaponToEquip == nullptr) return;
	SecondaryWeapon = WeaponToEquip;
	SecondaryWeapon->SetWeaponState(EWeaponState::EWS_EquippedSecondary);
	SecondaryWeapon->SetOwner(Character);
	AttachActorToBackpack(WeaponToEquip);
	PlayEquipWeaponSound(WeaponToEquip);
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
	if (!EquippedWeapon->IsEmpty() && bCanFire && CombatState == ECombatState::ECS_Reloading && EquippedWeapon->GetWeaponType() == EWeaponType::EWT_Shotgun)
	{
		return true;
	}
	if (bLocallyReloading)
	{
		return false;
	}
	return !EquippedWeapon->IsEmpty() && bCanFire && CombatState == ECombatState::ECS_Unoccupied;
}

void UCombatComponent::DropEquippedWeapon()
{
	if (EquippedWeapon)
	{
		EquippedWeapon->Dropped();
	}
}

bool UCombatComponent::AttachActorToRightHand(AActor* ActorToAttach)
{
	bool bResult = false; 
	const USkeletalMeshSocket* HandSocket = Character->GetMesh()->GetSocketByName(TEXT("RightHandSocket"));
	if (HandSocket)
	{
		bResult = HandSocket->AttachActor(EquippedWeapon, Character->GetMesh());
		if (bResult)
		{
			EquippedWeapon->ShowPickupWidget(false);
			EquippedWeapon->SetOwner(Character);
			EquippedWeapon->SetHUDAmmo();
		}
	}
	return bResult;
}

bool UCombatComponent::AttachActorToLeftHand(AActor* ActorToAttach)
{
	if (!EquippedWeapon)
	{
		return false;
	}
	bool bResult = false;
	const bool bUsePistolSocket = EquippedWeapon->GetWeaponType() == EWeaponType::EWT_Pistol || EquippedWeapon->GetWeaponType() == EWeaponType::EWT_SubmachineGun;
	const FName SocketName = bUsePistolSocket ? TEXT("PistolSocket") : TEXT("LeftHandSocket"); 
	const USkeletalMeshSocket* HandSocket = Character->GetMesh()->GetSocketByName(SocketName);
	if (HandSocket)
	{
		bResult = HandSocket->AttachActor(EquippedWeapon, Character->GetMesh());
		if (bResult)
		{
			EquippedWeapon->ShowPickupWidget(false);
			EquippedWeapon->SetOwner(Character);
			EquippedWeapon->SetHUDAmmo();
		}
	}
	return bResult;
}

bool UCombatComponent::AttachActorToBackpack(AActor* ActorToAttach)
{
	if (Character == nullptr || Character->GetMesh() == nullptr || ActorToAttach == nullptr)
	{
		return false;
	}
	const USkeletalMeshSocket* BackpackSocket = Character->GetMesh()->GetSocketByName(TEXT("BackpackSocket"));
	if (BackpackSocket)
	{
		return BackpackSocket->AttachActor(ActorToAttach, Character->GetMesh());
	}
	return false;
}

bool UCombatComponent::AttachFlagToLeftHand(AWeaponBase* FlagToAttach)
{
	bool bResult = false; 
	const USkeletalMeshSocket* HandSocket = Character->GetMesh()->GetSocketByName(TEXT("FlagSocket"));
	if (HandSocket)
	{
		bResult = HandSocket->AttachActor(FlagToAttach, Character->GetMesh());
		if (bResult)
		{
			FlagToAttach->ShowPickupWidget(false);
			FlagToAttach->SetOwner(Character);
		}
	}
	return bResult;
}

void UCombatComponent::UpdateCarriedAmmo()
{
	if (EquippedWeapon == nullptr)
	{
		return;
	}
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
}

void UCombatComponent::PlayEquipWeaponSound(AWeaponBase* WeaponToEquip)
{
	if (WeaponToEquip->EquipSoundFX)
	{
		UGameplayStatics::PlaySoundAtLocation(this, WeaponToEquip->EquipSoundFX, WeaponToEquip->GetActorLocation());
	}
}

void UCombatComponent::ReloadEmptyWeapon()
{
	if (EquippedWeapon->IsEmpty())
	{
		Reload();
	}
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

void UCombatComponent::ThrowGrenade()
{
	if (Grenades == 0)
	{
		return;
	}
	if (CombatState != ECombatState::ECS_Unoccupied || EquippedWeapon == nullptr)
	{
		return;
	}
	CombatState = ECombatState::ECS_ThrowingGrenade;
	if (Character)
	{
		Character->PlayThrowGrenadeMontage();
		AttachActorToLeftHand(EquippedWeapon);
		ShowAttachedGrenade(true);
		if (!Character->HasAuthority())
		{
			ServerThrowGrenade();
		}
		if (Character->HasAuthority())
		{
			Grenades = FMath::Clamp(Grenades - 1, 0, MaxGrenades);
			UpdateHUDGrenades();
		}
	}
}

void UCombatComponent::ShowAttachedGrenade(const bool bShowGrenade)
{
	if (Character && Character->GetAttachedGrenadeMesh())
	{
		Character->GetAttachedGrenadeMesh()->SetVisibility(bShowGrenade);
	}
}

void UCombatComponent::ServerThrowGrenade_Implementation()
{
	CombatState = ECombatState::ECS_ThrowingGrenade;
	if (Character)
	{
		Character->PlayThrowGrenadeMontage();
		AttachActorToLeftHand(EquippedWeapon);
		ShowAttachedGrenade(true);
	}
	Grenades = FMath::Clamp(Grenades - 1, 0, MaxGrenades);
	UpdateHUDGrenades();
}

void UCombatComponent::ServerReload_Implementation()
{
	if (Character == nullptr || EquippedWeapon == nullptr)
	{
		return;
	}

	CombatState = ECombatState::ECS_Reloading;
	if (!Character->IsLocallyControlled())
	{
		HandleReload();	
	}
}

void UCombatComponent::OnRep_EquippedWeapon()
{
	if (EquippedWeapon == nullptr || Character == nullptr)
	{
		return;
	}

	EquippedWeapon->SetWeaponState(EWeaponState::EWS_Equipped);
	EquippedWeapon->EnableCustomDepth(false);
	AttachActorToRightHand(EquippedWeapon);
	PlayEquipWeaponSound(EquippedWeapon);
	Character->GetCharacterMovement()->bOrientRotationToMovement = false;
	Character->bUseControllerRotationYaw = true;
	EquippedWeapon->SetHUDAmmo();
}

void UCombatComponent::OnRep_SecondaryWeapon()
{
	if (SecondaryWeapon && Character)
	{
		SecondaryWeapon->SetWeaponState(EWeaponState::EWS_EquippedSecondary);
		AttachActorToBackpack(SecondaryWeapon);
		PlayEquipWeaponSound(EquippedWeapon);
	}
}

void UCombatComponent::OnRep_CarriedAmmo()
{
	if (Character == nullptr) return;
	
	Controller = Controller == nullptr ? Cast<ABlasterPlayerController>(Character->Controller) : Controller;
	if (Controller)
	{
		Controller->SetHUDCarriedAmmo(CarriedAmmo);
	}
	bool bJumpToShotgunEnd = CombatState == ECombatState::ECS_Reloading &&
		EquippedWeapon &&
			EquippedWeapon->GetWeaponType() == EWeaponType::EWT_Shotgun &&
				CarriedAmmo == 0;
	if (bJumpToShotgunEnd)
	{
		JumpToShotgunEnd();
	}
}

void UCombatComponent::OnRep_CombatState()
{
	switch (CombatState)
	{
		case ECombatState::ECS_Reloading:
			if (!Character->IsLocallyControlled())
			{
				HandleReload();
			}
			break;
		case ECombatState::ECS_Unoccupied:
			if (bFireButtonPressed)
			{
				Fire();
			}
			break;
	case ECombatState::ECS_ThrowingGrenade:
		if (Character && !Character->IsLocallyControlled())
		{
			Character->PlayThrowGrenadeMontage();
			AttachActorToLeftHand(EquippedWeapon);
			ShowAttachedGrenade(true);
		}
		break;
	case ECombatState::ECS_SwappingWeapons:
		if (Character && !Character->IsLocallyControlled())
		{
			Character->PlaySwapMontage();
		}
		break;
	}
}

void UCombatComponent::OnRep_Grenades()
{
	UpdateHUDGrenades();
}

void UCombatComponent::OnRep_Aiming()
{
	if (Character->IsLocallyControlled())
	{
		bAiming = bAimButtonPressed;
	}
}

void UCombatComponent::OnRep_HoldingTheFlag()
{
	if (bHoldingTheFlag && Character && Character->IsLocallyControlled())
	{
		Character->Crouch();
	}
}

void UCombatComponent::ServerLaunchGrenade_Implementation(const FVector_NetQuantize& Target)
{
	if (Character && GrenadeClass && Character->GetAttachedGrenadeMesh())
	{
		const FVector StartingLocation = Character->GetAttachedGrenadeMesh()->GetComponentLocation();
		const FVector ToTarget = Target - StartingLocation;
		FActorSpawnParameters SpawnParams;
		SpawnParams.Owner = Character;
		SpawnParams.Instigator = Character;
		if (UWorld* World = GetWorld())
		{
			World->SpawnActor<AProjectileBase>(
				GrenadeClass,
				StartingLocation,
				ToTarget.Rotation(),
				SpawnParams
			);
		}
	}
}

void UCombatComponent::ServerShotgunFire_Implementation(const TArray<FVector_NetQuantize>& TraceHitTargets, float FireDelay)
{
	MulticastShotgunFire(TraceHitTargets);
}

bool UCombatComponent::ServerShotgunFire_Validate(const TArray<FVector_NetQuantize>& TraceHitTargets, float FireDelay)
{
	if (EquippedWeapon)
	{
		const bool bNearlyEqual = FMath::IsNearlyEqual(EquippedWeapon->FireDelay, FireDelay, 0.001f);
		return bNearlyEqual;
	}
	return true;
}

void UCombatComponent::MulticastShotgunFire_Implementation(const TArray<FVector_NetQuantize>& TraceHitTargets)
{
	if (Character && Character->IsLocallyControlled() && !Character->HasAuthority())
	{
		return;
	}
	ShotgunLocalFire(TraceHitTargets);
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
		if (EquippedWeapon)
		{
			CrosshairShootingFactor = 0.75f;
			switch (EquippedWeapon->GetFireType())
			{
			case EFireType::EFT_Projectile:
				FireProjectileWeapon();
				break;
			case EFireType::EFT_HitScan:
				FireHitScanWeapon();
				break;
			case EFireType::EFT_Shotgun:
				FireShotgun();
				break;
			}
		}
		StartFireTimer();
	}
}

void UCombatComponent::LocalFire(const FVector_NetQuantize& TraceHitTarget)
{
	if (Character && CombatState == ECombatState::ECS_Reloading && EquippedWeapon->GetWeaponType() == EWeaponType::EWT_Shotgun)
	{
		Character->PlayFireMontage(bAiming);
		CombatState = ECombatState::ECS_Unoccupied;
		EquippedWeapon->Fire(TraceHitTarget);
		return;
	}
	if (EquippedWeapon && Character && CombatState == ECombatState::ECS_Unoccupied)
	{
		Character->PlayFireMontage(bAiming);
		EquippedWeapon->Fire(TraceHitTarget);
	}
}

void UCombatComponent::ShotgunLocalFire(const TArray<FVector_NetQuantize>& TraceHitTargets)
{
	if (EquippedWeapon == nullptr || Character == nullptr) return;
	AShotgunWeapon* Shotgun = Cast<AShotgunWeapon>(EquippedWeapon);
	if (Shotgun && (CombatState == ECombatState::ECS_Reloading || CombatState == ECombatState::ECS_Unoccupied))
	{
		bLocallyReloading = false;
		Character->PlayFireMontage(bAiming);
		Shotgun->FireShotgun(TraceHitTargets);
	}
}

void UCombatComponent::FireProjectileWeapon()
{
	if (EquippedWeapon && Character)
	{
		HitTarget = EquippedWeapon->bUseScatter ? EquippedWeapon->TraceEndWithScatter(HitTarget) : HitTarget;
		if (!Character->HasAuthority()) LocalFire(HitTarget);
		ServerFire(HitTarget, EquippedWeapon->FireDelay);
	}
}

void UCombatComponent::FireHitScanWeapon()
{
	if (EquippedWeapon && Character)
	{
		HitTarget = EquippedWeapon->bUseScatter ? EquippedWeapon->TraceEndWithScatter(HitTarget) : HitTarget;
		if (!Character->HasAuthority()) LocalFire(HitTarget);
		ServerFire(HitTarget, EquippedWeapon->FireDelay);
	}
}

void UCombatComponent::FireShotgun()
{
	AShotgunWeapon* Shotgun = Cast<AShotgunWeapon>(EquippedWeapon);
	if (Shotgun && Character)
	{
		TArray<FVector_NetQuantize> HitTargets;
		Shotgun->ShotgunTraceEndWithScatter(HitTarget, HitTargets);
		if (!Character->HasAuthority()) ShotgunLocalFire(HitTargets);
		ServerShotgunFire(HitTargets, EquippedWeapon->FireDelay);
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
	EquippedWeapon->AddAmmo(ReloadAmount);
}

void UCombatComponent::UpdateShotgunAmmoValues()
{
	if (Character == nullptr || EquippedWeapon == nullptr)
	{
		return;
	}

	if (CarriedAmmoMap.Contains(EquippedWeapon->GetWeaponType()))
	{
		CarriedAmmoMap[EquippedWeapon->GetWeaponType()] -= 1;
		CarriedAmmo = CarriedAmmoMap[EquippedWeapon->GetWeaponType()];
	}
	Controller = Controller == nullptr ? Cast<ABlasterPlayerController>(Character->Controller) : Controller;
	if (Controller)
	{
		Controller->SetHUDCarriedAmmo(CarriedAmmo);
	}
	EquippedWeapon->AddAmmo(1);
	bCanFire = true;
	if (EquippedWeapon->IsFull() || CarriedAmmo == 0)
	{
		JumpToShotgunEnd();
	}
}

void UCombatComponent::MulticastFire_Implementation(const FVector_NetQuantize& TraceHitTarget)
{
	if (Character && Character->IsLocallyControlled() && !Character->HasAuthority())
	{
		return;
	}
	LocalFire(TraceHitTarget);
}

void UCombatComponent::ServerFire_Implementation(const FVector_NetQuantize& TraceHitTarget, float FireDelay)
{
	MulticastFire(TraceHitTarget);
}

bool UCombatComponent::ServerFire_Validate(const FVector_NetQuantize& TraceHitTarget, float FireDelay)
{
	if (EquippedWeapon)
	{
		const bool bNearlyEqual = FMath::IsNearlyEqual(EquippedWeapon->FireDelay, FireDelay, 0.001f);
		return bNearlyEqual;
	}
	return true;
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
	if (CarriedAmmo > 0 && CombatState == ECombatState::ECS_Unoccupied && EquippedWeapon && !EquippedWeapon->IsFull())
	{
		ServerReload();
		HandleReload();
		bLocallyReloading = true;
	}
}

void UCombatComponent::FinishReloading()
{
	if (Character && Character->HasAuthority())
	{
		CombatState = ECombatState::ECS_Unoccupied;
		UpdateAmmoValues();
	}
	bLocallyReloading = false;
	if (bFireButtonPressed)
	{
		Fire();
	}
}

void UCombatComponent::ShotgunShellReload()
{
	if (Character && Character->HasAuthority())
	{
		UpdateShotgunAmmoValues();
	}
}

void UCombatComponent::ThrowGrenadeFinished()
{
	CombatState = ECombatState::ECS_Unoccupied;
	AttachActorToRightHand(EquippedWeapon);
}

void UCombatComponent::LaunchGrenade()
{
	ShowAttachedGrenade(false);
	if (Character && Character->IsLocallyControlled())
	{
		ServerLaunchGrenade(HitTarget);
	}
}

void UCombatComponent::FinishSwap()
{
	if (Character && Character->HasAuthority())
	{
		CombatState = ECombatState::ECS_Unoccupied;
	}
	if (Character)
	{
		Character->bFinishedSwapping = true;
	}
	if (SecondaryWeapon)
	{
		SecondaryWeapon->EnableCustomDepth(true);
	}
}

void UCombatComponent::FinishSwapAttachWeapons()
{
	EquippedWeapon->SetWeaponState(EWeaponState::EWS_Equipped);
	AttachActorToRightHand(EquippedWeapon);
	EquippedWeapon->SetHUDAmmo();
	UpdateCarriedAmmo();
	PlayEquipWeaponSound(EquippedWeapon);

	SecondaryWeapon->SetWeaponState(EWeaponState::EWS_EquippedSecondary);
	AttachActorToBackpack(SecondaryWeapon);
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
				HUDPackage.CrosshairCenter = EquippedWeapon->CrosshairCenter ? EquippedWeapon->CrosshairCenter : nullptr;
				HUDPackage.CrosshairLeft = EquippedWeapon->CrosshairLeft ? EquippedWeapon->CrosshairLeft : nullptr;
				HUDPackage.CrosshairRight = EquippedWeapon->CrosshairRight ? EquippedWeapon->CrosshairRight : nullptr;
				HUDPackage.CrosshairTop = EquippedWeapon->CrosshairTop ? EquippedWeapon->CrosshairTop : nullptr;
				HUDPackage.CrosshairBottom = EquippedWeapon->CrosshairBottom ? EquippedWeapon->CrosshairBottom : nullptr;
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

void UCombatComponent::JumpToShotgunEnd()
{
	if (Character == nullptr)
	{
		return;
	}
	
	UAnimInstance* AnimInstance = Character->GetMesh()->GetAnimInstance();
	if (AnimInstance && Character->GetReloadMontage())
	{
		AnimInstance->Montage_Play(Character->GetReloadMontage());
		const FName SectionName = TEXT("ShotgunEnd");
		AnimInstance->Montage_JumpToSection(SectionName, Character->GetReloadMontage());
	}
}

void UCombatComponent::UpdateHUDGrenades()
{
	Controller = Controller == nullptr ? Cast<ABlasterPlayerController>(Character->Controller) : Controller;
	if (Controller)
	{
		Controller->SetHUDGrenades(Grenades);
	}
}

void UCombatComponent::PickupAmmo(EWeaponType WeaponType, int32 AmmoAmount)
{
	if (CarriedAmmoMap.Contains(WeaponType))
	{
		CarriedAmmoMap[WeaponType] = FMath::Clamp(CarriedAmmoMap[WeaponType] + AmmoAmount, 0, MaxCarriedAmmo);
		UpdateCarriedAmmo();
	}
	if (EquippedWeapon && EquippedWeapon->IsEmpty() && EquippedWeapon->GetWeaponType() == WeaponType)
	{
		Reload();
	}
}

void UCombatComponent::SwapWeapon()
{
	if (CombatState != ECombatState::ECS_Unoccupied || Character == nullptr) return;

	Character->PlaySwapMontage();
	Character->bFinishedSwapping = false;
	CombatState = ECombatState::ECS_SwappingWeapons;
	
	AWeaponBase* TempWeapon = EquippedWeapon;
	EquippedWeapon = SecondaryWeapon;
	SecondaryWeapon = TempWeapon;

	if (SecondaryWeapon)
	{
		SecondaryWeapon->EnableCustomDepth(false);
	}
}

bool UCombatComponent::ShouldSwapWeapon()
{
	return (EquippedWeapon != nullptr && SecondaryWeapon != nullptr);
}

void UCombatComponent::InitializeCarriedAmmo()
{
	CarriedAmmoMap.Emplace(EWeaponType::EWT_AssaultRifle, StartingARAmmo);
	CarriedAmmoMap.Emplace(EWeaponType::EWT_RocketLauncher, StartingRocketAmmo);
	CarriedAmmoMap.Emplace(EWeaponType::EWT_Pistol, StartingPistolAmmo);
	CarriedAmmoMap.Emplace(EWeaponType::EWT_SubmachineGun, StartingSubmachineGunAmmo);
	CarriedAmmoMap.Emplace(EWeaponType::EWT_Shotgun, StartingShotgunAmmo);
	CarriedAmmoMap.Emplace(EWeaponType::EWT_SniperRifle, StartingSniperRifleAmmo);
	CarriedAmmoMap.Emplace(EWeaponType::EWT_GrenadeLauncher, StartingGrenadeLauncherAmmo);
}
