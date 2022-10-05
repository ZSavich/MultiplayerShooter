// Fill out your copyright notice in the Description page of Project Settings.

#include "Zones/FlagZone.h"

#include "GameModes/CaptureTheFlagGameMode.h"
#include "Weapons/Flag.h"

AFlagZone::AFlagZone()
{
 	PrimaryActorTick.bCanEverTick = false;
	ZoneSphere = CreateDefaultSubobject<USphereComponent>(TEXT("Zone Sphere Component"));
	SetRootComponent(ZoneSphere);
}

void AFlagZone::BeginPlay()
{
	Super::BeginPlay();
	ZoneSphere->OnComponentBeginOverlap.AddDynamic(this, &AFlagZone::OnSphereOverlap);
}

void AFlagZone::OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	AFlag* OverlappingFlag = Cast<AFlag>(OtherActor);
	if (OverlappingFlag && OverlappingFlag->GetTeam() != Team)
	{
		ACaptureTheFlagGameMode* CTFGameMode = GetWorld()->GetAuthGameMode<ACaptureTheFlagGameMode>();
		if (CTFGameMode)
		{
			CTFGameMode->FlagCaptured(OverlappingFlag, this);
		}
		OverlappingFlag->ResetFlag();
	}
}
