// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "WFC3DActor.generated.h"

UCLASS()
class PROCEDURALWORLD_API AWFC3DActor : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	AWFC3DActor();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;
};
