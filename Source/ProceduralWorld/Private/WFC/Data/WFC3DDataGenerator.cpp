// Fill out your copyright notice in the Description page of Project Settings.


#include "WFC/Data/WFC3DDataGenerator.h"


// Sets default values
AWFC3DDataGenerator::AWFC3DDataGenerator()
{
	PrimaryActorTick.bCanEverTick = false;
}

void AWFC3DDataGenerator::OnConstruction(const FTransform& Transform)
{
	if (bCreateModelData)
	{
		bool bResult = CreateModelData();
		UE_LOG(LogTemp, Display, TEXT("Create Model Data: %s"), bResult ? TEXT("Success") : TEXT("Failed"));
		bCreateModelData = false;
	}

	if (bPrintModelData)
	{
		bool bResult = PrintModelData();
		UE_LOG(LogTemp, Display, TEXT("Print Model Data: %s"), bResult ? TEXT("Success") : TEXT("Failed"));
		bPrintModelData = false;
	}
}

bool AWFC3DDataGenerator::CreateModelData() const
{
	// Create Model Data
	return WFC3DModel->InitializeData();
}

bool AWFC3DDataGenerator::PrintModelData() const
{
	// Print Model Data
	return WFC3DModel->PrintData();
}
