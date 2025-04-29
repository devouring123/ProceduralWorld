// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "WFC/Interface/WFC3DAlgorithmInterface.h"

struct FFaceInfo;
struct FTileInfo;
/**
 * 
 */
USTRUCT(BlueprintType)
struct PROCEDURALWORLD_API FWFC3DAlgorithmData
{
	GENERATED_BODY()

public:
	FWFC3DAlgorithmData(const IWFC3DAlgorithmInterface* InAlgorithmInterface)
		: AlgorithmInterface(InAlgorithmInterface),
		  TileInfos(InAlgorithmInterface->GetTileInfos()),
		  FaceInfos(InAlgorithmInterface->GetFaceInfos())
	{
	}

	/** Initialize */
	bool InitializeData();

private:
	const IWFC3DAlgorithmInterface* AlgorithmInterface = nullptr;
	const TArray<FTileInfo>* TileInfos;
	const TArray<FFaceInfo>* FaceInfos;
};
