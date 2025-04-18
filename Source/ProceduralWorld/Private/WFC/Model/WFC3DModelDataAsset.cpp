// Fill out your copyright notice in the Description page of Project Settings.


#include "WFC/Model/WFC3DModelDataAsset.h"

bool UWFC3DModelDataAsset::InitializeAlgorithmData()
{
	/** Common Init */
	// Success &= InitializeTileInfo();
	// Success &= InitializeFaceInfo();
	
	bool Success = true;
	Success &= InitializeFaceToTileBitArray();
	Success &= InitializeFaceToTileBitString();

}

const TArray<FTileInfo>& UWFC3DModelDataAsset::GetTileInfos() const
{
}

const TArray<FFaceInfo>& UWFC3DModelDataAsset::GetFaceInfos() const
{
}

const TBitArray<>& UWFC3DModelDataAsset::GetCompatibleTiles(const int32& FaceIndex) const
{
}

const FTileFaceIndices& UWFC3DModelDataAsset::GetTileFaceIndices(const int32& TileIndex) const
{
}

const float& UWFC3DModelDataAsset::GetTileWeight(const int32& TileIndex) const
{
}

bool UWFC3DModelDataAsset::InitializeVisualizationData()
{
	bool Success = true;
	Success &= InitializeTileVariantInfo();
	Success &= InitializeTileRotationInfo();	
}

const TArray<FTileRotationInfo>& UWFC3DModelDataAsset::GetTileRotationInfo() const
{
}

const FTileVariantInfo& UWFC3DModelDataAsset::GetTileVariant(const int32& TileIndex) const
{
}

const FTileVisualInfo& UWFC3DModelDataAsset::GetRandomTileVisualInfo(const int32& BaseTileIndex, const FString& BiomeName) const
{
}

bool UWFC3DModelDataAsset::InitializeTileInfo()
{
}

bool UWFC3DModelDataAsset::InitializeFaceInfo()
{
}

bool UWFC3DModelDataAsset::InitializeFaceToTileBitArray()
{
}

bool UWFC3DModelDataAsset::InitializeFaceToTileBitString()
{
}

bool UWFC3DModelDataAsset::InitializeTileVariantInfo()
{
}

bool UWFC3DModelDataAsset::InitializeTileRotationInfo()
{
}
