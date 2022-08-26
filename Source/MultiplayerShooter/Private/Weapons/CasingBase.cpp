// Fill out your copyright notice in the Description page of Project Settings.

#include "Weapons/CasingBase.h"

#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "Sound/SoundCue.h"

ACasingBase::ACasingBase()
{
 	PrimaryActorTick.bCanEverTick = false;
	ShellEjectionImpulse = 5.f;

	ShellMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ShellMesh"));
	SetRootComponent(ShellMesh);
	ShellMesh->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);
	ShellMesh->SetNotifyRigidBodyCollision(true);
	ShellMesh->SetSimulatePhysics(true);
	ShellMesh->SetEnableGravity(true);
}

void ACasingBase::BeginPlay()
{
	Super::BeginPlay();

	ShellMesh->OnComponentHit.AddDynamic(this, &ACasingBase::OnHit);
	const FVector RandomShellDirection = UKismetMathLibrary::RandomUnitVectorInConeInDegrees(
		GetActorForwardVector(),
		20.f
	);
	ShellMesh->AddImpulse(RandomShellDirection * ShellEjectionImpulse);
}

void ACasingBase::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp,
	FVector NormalImpulse, const FHitResult& Hit)
{
	if (ShellSoundFX)
	{
		UGameplayStatics::PlaySoundAtLocation(
			GetWorld(),
			ShellSoundFX,
			GetActorLocation()
		);
	}
	ShellMesh->SetNotifyRigidBodyCollision(false);
	SetLifeSpan(3.f);
}
