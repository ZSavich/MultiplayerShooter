// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapons/Flag.h"

#include "Character/BlasterCharacter.h"
#include "Components/SphereComponent.h"
#include "Components/WidgetComponent.h"
#include "Weapons/WeaponTypes.h"

AFlag::AFlag()
{
	FlagMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Flag Mesh"));
	SetRootComponent(FlagMesh);
	FlagMesh->SetCollisionResponseToAllChannels(ECR_Ignore);
	FlagMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	
	AreaSphere->SetupAttachment(FlagMesh);
	GetPickupWidget()->SetupAttachment(FlagMesh);
}

void AFlag::Dropped()
{
	SetWeaponState(EWeaponState::EWS_Dropped);
	const FDetachmentTransformRules DetachRules(EDetachmentRule::KeepWorld, true);
	FlagMesh->DetachFromComponent(DetachRules);
	SetOwner(nullptr);
	BlasterOwnerCharacter = nullptr;
	BlasterOwnerController = nullptr;
}

void AFlag::OnDropped()
{
	if (!FlagMesh) return;
	if (HasAuthority())
	{
		AreaSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	}
	FlagMesh->SetSimulatePhysics(true);
	FlagMesh->SetEnableGravity(true);
	FlagMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	FlagMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Block);
	FlagMesh->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore);
	FlagMesh->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);
	FlagMesh->SetCustomDepthStencilValue(CUSTOM_DEPTH_PURPLE);
	FlagMesh->MarkRenderStateDirty();
	EnableCustomDepth(true);
}

void AFlag::OnEquipped()
{
	if (!FlagMesh) return;
	ShowPickupWidget(false);
	AreaSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	FlagMesh->SetSimulatePhysics(false);
	FlagMesh->SetEnableGravity(false);
	FlagMesh->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	FlagMesh->SetCollisionResponseToChannel(ECC_WorldDynamic, ECR_Overlap);
	EnableCustomDepth(false);
}

void AFlag::BeginPlay()
{
	Super::BeginPlay();
	InitialTransform = GetActorTransform();
}

void AFlag::ResetFlag()
{
	SetWeaponState(EWeaponState::EWS_Dropped);

	if (!HasAuthority())
	{
		const FDetachmentTransformRules DetachRules(EDetachmentRule::KeepWorld, true);
		FlagMesh->DetachFromComponent(DetachRules);
	}

	if (ABlasterCharacter* FlagBearer = Cast<ABlasterCharacter>(Owner))
	{
		FlagBearer->SetHoldingTheFlag(false);
		FlagBearer->SetOverlappingWeapon(nullptr);
		FlagBearer->UnCrouch();
	}

	SetWeaponState(EWeaponState::EWS_Initial);
	AreaSphere->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	AreaSphere->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	
	SetOwner(nullptr);
	BlasterOwnerCharacter = nullptr;
	BlasterOwnerController = nullptr;
	SetActorTransform(InitialTransform);
}
