// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapons/WeaponBase.h"

#include "Character/BlasterCharacter.h"
#include "Components/SphereComponent.h"
#include "Components/WidgetComponent.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Net/UnrealNetwork.h"
#include "Character/BlasterCharacter.h"
#include "Character/BlasterPlayerController.h"
#include "Kismet/KismetMathLibrary.h"

AWeaponBase::AWeaponBase()
{
 	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;
	SetReplicateMovement(true);

	WeaponMeshComponent = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("WeaponMesh"));
	SetRootComponent(WeaponMeshComponent);
	WeaponMeshComponent->SetCollisionResponseToAllChannels(ECR_Block);
	WeaponMeshComponent->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore);
	WeaponMeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	WeaponMeshComponent->SetCustomDepthStencilValue(CUSTOM_DEPTH_BLUE);
	WeaponMeshComponent->MarkRenderStateDirty();
	EnableCustomDepth(true);

	AreaSphere = CreateDefaultSubobject<USphereComponent>(TEXT("AreaSphere"));
	AreaSphere->SetupAttachment(RootComponent);
	AreaSphere->SetCollisionResponseToAllChannels(ECR_Ignore);
	AreaSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	PickupWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("PickupWidget"));
	PickupWidget->SetupAttachment(RootComponent);
}

void AWeaponBase::BeginPlay()
{
	Super::BeginPlay();

	AreaSphere->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	AreaSphere->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	AreaSphere->OnComponentBeginOverlap.AddDynamic(this, &AWeaponBase::OnSphereOverlapped);
	AreaSphere->OnComponentEndOverlap.AddDynamic(this, &AWeaponBase::OnSphereEndOverlap);
	
	if (PickupWidget)
	{
		PickupWidget->SetVisibility(false);
	}
}

void AWeaponBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AWeaponBase, WeaponState);
	DOREPLIFETIME_CONDITION(AWeaponBase, bUseServerSideRewind, COND_OwnerOnly);
}

bool AWeaponBase::IsEmpty()
{
	return Ammo <= 0;
}

bool AWeaponBase::IsFull()
{
	return Ammo == MagCapacity;
}

void AWeaponBase::OnSphereOverlapped(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
                                     bool bFromSweep, const FHitResult& SweepResult)
{
	if (ABlasterCharacter* BlasterCharacter = Cast<ABlasterCharacter>(OtherActor))
	{
		if (WeaponType == EWeaponType::EWT_Flag && BlasterCharacter->GetTeam() == Team)
		{
			return;
		}
		if (BlasterCharacter->IsHoldingTheFlag())
		{
			return;
		}
		BlasterCharacter->SetOverlappingWeapon(this);
	}
}

void AWeaponBase::OnSphereEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (ABlasterCharacter* BlasterCharacter = Cast<ABlasterCharacter>(OtherActor))
	{
		if (WeaponType == EWeaponType::EWT_Flag && BlasterCharacter->GetTeam() == Team)
		{
			return;
		}
		if (BlasterCharacter->IsHoldingTheFlag())
		{
			return;
		}
		BlasterCharacter->SetOverlappingWeapon(nullptr);
	}
}

void AWeaponBase::ShowPickupWidget(bool IsVisible) const
{
	if (PickupWidget)
	{
		PickupWidget->SetVisibility(IsVisible);
	}
}

void AWeaponBase::EnableCustomDepth(const bool bEnable)
{
	if (WeaponMeshComponent)
	{
		WeaponMeshComponent->SetRenderCustomDepth(bEnable);
	}
}

void AWeaponBase::SetWeaponState(EWeaponState InWeaponState)
{
	WeaponState = InWeaponState;
	OnWeaponStateSet();
}

void AWeaponBase::OnRep_WeaponState()
{
	OnWeaponStateSet();
}

void AWeaponBase::ClientUpdateAmmo_Implementation(float ServerAmmo)
{
	if (HasAuthority()) return;
	Ammo = ServerAmmo;
	--Sequence;
	Ammo -= Sequence;
	SetHUDAmmo();
}

void AWeaponBase::OnWeaponStateSet()
{
	switch (WeaponState)
	{
	case EWeaponState::EWS_Equipped:
		OnEquipped();
		break;
	case EWeaponState::EWS_EquippedSecondary:
		OnEquippedSecondary();
		break;
	case EWeaponState::EWS_Dropped:
		OnDropped();
		break;
	}
}

void AWeaponBase::OnEquipped()
{
	if (!WeaponMeshComponent) return;
	ShowPickupWidget(false);
	AreaSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	WeaponMeshComponent->SetSimulatePhysics(false);
	WeaponMeshComponent->SetEnableGravity(false);
	WeaponMeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	if (WeaponType == EWeaponType::EWT_SubmachineGun)
	{
		WeaponMeshComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		WeaponMeshComponent->SetEnableGravity(true);
		WeaponMeshComponent->SetCollisionResponseToAllChannels(ECR_Ignore);
	}
	EnableCustomDepth(false);

	BlasterOwnerCharacter = BlasterOwnerCharacter == nullptr ? GetOwner<ABlasterCharacter>() : BlasterOwnerCharacter;
	if (BlasterOwnerCharacter && bUseServerSideRewind)
	{
		BlasterOwnerController = BlasterOwnerController == nullptr ? Cast<ABlasterPlayerController>(BlasterOwnerCharacter->Controller) : BlasterOwnerController;
		if (BlasterOwnerController && !BlasterOwnerController->HighPingDelegate.IsBound())
		{
			BlasterOwnerController->HighPingDelegate.AddDynamic(this, &AWeaponBase::OnPingTooHigh);
		}
	}
}

void AWeaponBase::OnDropped()
{
	if (!WeaponMeshComponent) return;
	if (HasAuthority())
	{
		AreaSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	}
	WeaponMeshComponent->SetSimulatePhysics(true);
	WeaponMeshComponent->SetEnableGravity(true);
	WeaponMeshComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	WeaponMeshComponent->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Block);
	WeaponMeshComponent->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore);
	WeaponMeshComponent->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);
	WeaponMeshComponent->SetCustomDepthStencilValue(CUSTOM_DEPTH_PURPLE);
	WeaponMeshComponent->MarkRenderStateDirty();
	EnableCustomDepth(true);
	
	BlasterOwnerCharacter = BlasterOwnerCharacter == nullptr ? GetOwner<ABlasterCharacter>() : BlasterOwnerCharacter;
	if (BlasterOwnerCharacter && bUseServerSideRewind)
	{
		BlasterOwnerController = BlasterOwnerController == nullptr ? Cast<ABlasterPlayerController>(BlasterOwnerCharacter->Controller) : BlasterOwnerController;
		if (BlasterOwnerController && BlasterOwnerController->HighPingDelegate.IsBound())
		{
			BlasterOwnerController->HighPingDelegate.RemoveDynamic(this, &AWeaponBase::OnPingTooHigh);
		}
	}
}

void AWeaponBase::OnEquippedSecondary()
{
	if (!WeaponMeshComponent) return;
	ShowPickupWidget(false);
	AreaSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	WeaponMeshComponent->SetSimulatePhysics(false);
	WeaponMeshComponent->SetEnableGravity(false);
	WeaponMeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	if (WeaponType == EWeaponType::EWT_SubmachineGun)
	{
		WeaponMeshComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		WeaponMeshComponent->SetEnableGravity(true);
		WeaponMeshComponent->SetCollisionResponseToAllChannels(ECR_Ignore);
	}
	WeaponMeshComponent->SetCustomDepthStencilValue(CUSTOM_DEPTH_TAN);
	WeaponMeshComponent->MarkRenderStateDirty();

	BlasterOwnerCharacter = BlasterOwnerCharacter == nullptr ? GetOwner<ABlasterCharacter>() : BlasterOwnerCharacter;
	if (BlasterOwnerCharacter && bUseServerSideRewind)
	{
		BlasterOwnerController = BlasterOwnerController == nullptr ? Cast<ABlasterPlayerController>(BlasterOwnerCharacter->Controller) : BlasterOwnerController;
		if (BlasterOwnerController && BlasterOwnerController->HighPingDelegate.IsBound())
		{
			BlasterOwnerController->HighPingDelegate.RemoveDynamic(this, &AWeaponBase::OnPingTooHigh);
		}
	}
}

void AWeaponBase::OnPingTooHigh(bool bPingTooHigh)
{
	bUseServerSideRewind = !bPingTooHigh;
}

void AWeaponBase::ClientAddAmmo_Implementation(float AmmoToAdd)
{
	if (HasAuthority()) return;
	Ammo = FMath::Clamp(Ammo + AmmoToAdd, 0.f, MagCapacity);
	BlasterOwnerCharacter = BlasterOwnerCharacter == nullptr ? GetOwner<ABlasterCharacter>() : BlasterOwnerCharacter;
	if (BlasterOwnerCharacter && BlasterOwnerCharacter->GetCombatComponent() && IsFull())
	{
		BlasterOwnerCharacter->GetCombatComponent()->JumpToShotgunEnd();
	}
	SetHUDAmmo();
}

void AWeaponBase::OnRep_Owner()
{
	Super::OnRep_Owner();
	if (Owner == nullptr)
	{
		BlasterOwnerCharacter = nullptr;
		BlasterOwnerController = nullptr;
	}
	else
	{
		BlasterOwnerCharacter = BlasterOwnerCharacter == nullptr ? Cast<ABlasterCharacter>(Owner) : BlasterOwnerCharacter;
		if (BlasterOwnerCharacter && BlasterOwnerCharacter->GetEquippedWeapon() && BlasterOwnerCharacter->GetEquippedWeapon() == this)
		{
			SetHUDAmmo();
		}
	}
}

void AWeaponBase::Fire(const FVector& HitTarget)
{
	if (FireAnimation && WeaponMeshComponent)
	{
		WeaponMeshComponent->PlayAnimation(FireAnimation, false);
	}
	if (CasingClass)
	{
		const USkeletalMeshSocket* AmmoEjectSocket = WeaponMeshComponent->GetSocketByName(TEXT("AmmoEject"));
		if (AmmoEjectSocket)
		{
			const FTransform SocketTransform = AmmoEjectSocket->GetSocketTransform(WeaponMeshComponent);

			if (UWorld* World = GetWorld())
			{
				World->SpawnActor<ACasingBase>(
					CasingClass,
					SocketTransform.GetLocation(),
					SocketTransform.GetRotation().Rotator()
				);
			}
		}
	}
	SpendRound();
}

void AWeaponBase::SetHUDAmmo()
{
	BlasterOwnerCharacter = BlasterOwnerCharacter == nullptr ? GetOwner<ABlasterCharacter>() : BlasterOwnerCharacter;
	if (BlasterOwnerCharacter)
	{
		BlasterOwnerController = BlasterOwnerController == nullptr ? Cast<ABlasterPlayerController>(BlasterOwnerCharacter->Controller) : BlasterOwnerController;
		if (BlasterOwnerController)
		{
			BlasterOwnerController->SetHUDWeaponAmmo(Ammo);
		}
	}
}

void AWeaponBase::Dropped()
{
	SetWeaponState(EWeaponState::EWS_Dropped);
	const FDetachmentTransformRules DetachRules(EDetachmentRule::KeepWorld, true);
	WeaponMeshComponent->DetachFromComponent(DetachRules);
	SetOwner(nullptr);
	BlasterOwnerCharacter = nullptr;
	BlasterOwnerController = nullptr;
}

void AWeaponBase::AddAmmo(const int32 AmmoToAdd)
{
	Ammo = FMath::Clamp(Ammo + AmmoToAdd, 0.f, MagCapacity);
	SetHUDAmmo();
	ClientAddAmmo(AmmoToAdd);
}

void AWeaponBase::SpendRound()
{
	Ammo = FMath::Clamp(Ammo - 1, 0, MagCapacity);
	SetHUDAmmo();
	if (HasAuthority())
	{
		ClientUpdateAmmo(Ammo);
	}
	else
	{
		++Sequence;
	}
}

FVector AWeaponBase::TraceEndWithScatter(const FVector& HitTarget)
{
	const USkeletalMeshSocket* MuzzleFlashSocket = GetWeaponMesh()->GetSocketByName(TEXT("MuzzleFlash"));
	if (!MuzzleFlashSocket) return FVector();
	
	const FTransform SocketTransform = MuzzleFlashSocket->GetSocketTransform(GetWeaponMesh());
	const FVector TraceStart = SocketTransform.GetLocation();
	const FVector ToTargetNormalized = (HitTarget - TraceStart).GetSafeNormal();
	const FVector SphereCenter = TraceStart + ToTargetNormalized * DistanceToSphere;

	const FVector RandVector = UKismetMathLibrary::RandomUnitVector() * FMath::FRandRange(0.f, SphereRadius);
	const FVector EndLoc = SphereCenter + RandVector;
	const FVector ToEndLoc = EndLoc - TraceStart;
	const FVector EndResult = FVector(TraceStart + ToEndLoc * TRACE_LENGTH / ToEndLoc.Size());

	/* 
	* Debug Scatter 
	DrawDebugSphere(GetWorld(), SphereCenter, SphereRadius, 12.f, FColor::Red, true);
	DrawDebugSphere(GetWorld(), EndLoc, 4.f, 12.f, FColor::Orange, true);
	DrawDebugLine(GetWorld(), TraceStart, EndResult, FColor::Cyan, true);
	*/
	
	return EndResult;
}