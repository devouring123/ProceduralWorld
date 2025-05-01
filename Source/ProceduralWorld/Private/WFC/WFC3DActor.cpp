// Fill out your copyright notice in the Description page of Project Settings.


#include "WFC/WFC3DActor.h"


// Sets default values
AWFC3DActor::AWFC3DActor()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}

// Called when the game starts or when spawned
void AWFC3DActor::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AWFC3DActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

