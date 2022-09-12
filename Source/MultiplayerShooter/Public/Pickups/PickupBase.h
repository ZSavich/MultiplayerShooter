// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "NiagaraComponent.h"
#include "GameFramework/Actor.h"
#include "PickupBase.generated.h"

class USoundCue;
class USphereComponent;

UCLASS()
class MULTIPLAYERSHOOTER_API APickupBase : public AActor
{
	GENERATED_BODY()

protected:
	UPROPERTY(EditAnywhere)
	float BaseTurnRate = 45.f;
	
private:
	UPROPERTY(EditAnywhere)
	USphereComponent* OverlapSphere;

	UPROPERTY(EditAnywhere)
	UStaticMeshComponent* PickupMesh;

	UPROPERTY(EditAnywhere)
	USoundCue* PickupSound;

	UPROPERTY(VisibleAnywhere, Category = "Components", meta = (AllowPrivateAccess = true))
	UNiagaraComponent* PickupEffectComponent;

	UPROPERTY(EditAnywhere)
	UNiagaraSystem* PickupEffect;

	FTimerHandle BindOverlapTimer;
	float BindOverlapTime = 0.25f;
	void BindOverlapTimerFinished();

public:	
	APickupBase();

	virtual void Destroyed() override;
	virtual void Tick(float DeltaSeconds) override;

protected:
	virtual void BeginPlay() override;

	UFUNCTION()
	virtual void OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult & SweepResult);

};
