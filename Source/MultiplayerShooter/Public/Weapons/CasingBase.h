// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CasingBase.generated.h"

class USoundCue;

UCLASS()
class MULTIPLAYERSHOOTER_API ACasingBase : public AActor
{
	GENERATED_BODY()
private:
	UPROPERTY(EditAnywhere, Category = "Shell Parameters")
	UStaticMeshComponent* ShellMesh;

	UPROPERTY(EditAnywhere, Category = "Shell Parameters")
	USoundCue* ShellSoundFX;

	UPROPERTY(EditAnywhere, Category = "Shell Parameters")
	float ShellEjectionImpulse;
	
public:	
	ACasingBase();

protected:
	virtual void BeginPlay() override;

	UFUNCTION()
	virtual void OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);
};
