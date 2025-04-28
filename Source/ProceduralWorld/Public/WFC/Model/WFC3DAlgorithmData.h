// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "WFC/Interface/WFC3DAlgorithmInterface.h"

struct FFaceInfo;
struct FTileInfo;
class IWFC3DAlgorithmInterface;
/**
 * 
 */
class PROCEDURALWORLD_API WFC3DAlgorithmData
{
public:
	WFC3DAlgorithmData() = default;
	virtual ~WFC3DAlgorithmData() = default;

	void Initialize(const IWFC3DAlgorithmInterface* InAlgorithmInterface)
	{
		AlgorithmInterface = InAlgorithmInterface;
		TileInfos = *AlgorithmInterface->GetTileInfos();
	}
	
private:
	const IWFC3DAlgorithmInterface* AlgorithmInterface = nullptr;
	TArray<FTileInfo> TileInfos;
	TArray<FFaceInfo> FaceInfos;
	TArray<TBitArray<>> FaceToTileBitArrays;

	
};
