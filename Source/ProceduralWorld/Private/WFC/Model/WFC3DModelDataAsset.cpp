// Fill out your copyright notice in the Description page of Project Settings.


#include "WFC/Model/WFC3DModelDataAsset.h"

bool UWFC3DModelDataAsset::InitializeData()
{
	bool bSuccess = true;
	bSuccess &= InitializeCommonData();
	bSuccess &= InitializeAlgorithmData();
	bSuccess &= InitializeVisualizationData();
	return bSuccess;
}

bool UWFC3DModelDataAsset::InitializeAlgorithmData()
{
	bool bSuccess = true;
	bSuccess &= InitializeFaceToTileBitArray();
	bSuccess &= InitializeFaceToTileBitString();
	return bSuccess;
}

const TArray<FTileInfo>* UWFC3DModelDataAsset::GetTileInfos() const
{
	if (TileInfos.IsEmpty())
	{
		UE_LOG(LogTemp, Error, TEXT("TileInfos is Empty"));
		return nullptr;
	}
	return &TileInfos;
}

const TArray<FFaceInfo>* UWFC3DModelDataAsset::GetFaceInfos() const
{
	if (FaceInfos.IsEmpty())
	{
		UE_LOG(LogTemp, Error, TEXT("FaceInfos is Empty"));
		return nullptr;
	}
	return &FaceInfos;
}

const TBitArray<>* UWFC3DModelDataAsset::GetCompatibleTiles(const int32& FaceIndex) const
{
	if (FaceToTileBitArrays.IsEmpty())
	{
		UE_LOG(LogTemp, Error, TEXT("FaceToTileBitArrays is Empty"));
		return nullptr;
	}
	if (FaceIndex < 0 || FaceIndex >= FaceToTileBitArrays.Num())
	{
		UE_LOG(LogTemp, Error, TEXT("FaceIndex is out of range"));
		return nullptr;
	}
	return &FaceToTileBitArrays[FaceIndex];
}

const float UWFC3DModelDataAsset::GetTileWeight(const int32& TileIndex) const
{
	if (TileInfos.IsEmpty())
	{
		UE_LOG(LogTemp, Error, TEXT("TileInfos is Empty"));
		return 0.0f;
	}
	return TileInfos[TileIndex].Weight;
}

bool UWFC3DModelDataAsset::InitializeVisualizationData()
{
	bool bSuccess = true;
	bSuccess &= InitializeTileVariantInfo();
	bSuccess &= InitializeTileRotationInfo();
	return bSuccess;
}

const TArray<FTileRotationInfo>* UWFC3DModelDataAsset::GetTileRotationInfo() const
{
	if (TileRotationInfos.IsEmpty())
	{
		UE_LOG(LogTemp, Error, TEXT("TileRotationInfos is Empty"));
		return nullptr;
	}
	return &TileRotationInfos;
}

const FTileVariantInfo* UWFC3DModelDataAsset::GetTileVariant(const int32& TileIndex) const
{
	if (TileVariants.IsEmpty())
	{
		UE_LOG(LogTemp, Error, TEXT("TileVariants is Empty"));
		return nullptr;
	}
	if (TileIndex < 0 || TileIndex >= TileVariants.Num())
	{
		UE_LOG(LogTemp, Error, TEXT("TileIndex is out of range"));
		return nullptr;
	}
	return &TileVariants[TileIndex];
}

const FTileVisualInfo* UWFC3DModelDataAsset::GetRandomTileVisualInfo(const int32& BaseTileIndex, const FString& BiomeName) const
{
	return nullptr;
}

bool UWFC3DModelDataAsset::InitializeCommonData()
{
	bool bSuccess = true;

	bSuccess &= InitializeBaseTileInfo();
	bSuccess &= InitializeFaceInfo();
	bSuccess &= InitializeTileInfo();

	return bSuccess;
}

bool UWFC3DModelDataAsset::InitializeBaseTileInfo()
{
	if (BaseTileDataTable == nullptr)
	{
		UE_LOG(LogTemp, Error, TEXT("BaseTileDataTable is nullptr"));
		return false;
	}

	BaseTileNameToIndex.Empty();
	BaseTileInfos.Empty();
	
	const TMap<FName, uint8*>& RowMap = BaseTileDataTable->GetRowMap();

	// RowMap 순회
	for (const TPair<FName, uint8*>& Row : RowMap)
	{
		if (const FTileInfoTable* RowData = reinterpret_cast<const FTileInfoTable*>(Row.Value))
		{
			BaseTileNameToIndex.Add(Row.Key.ToString(), BaseTileInfos.Num());			
			BaseTileInfos.Emplace(
				RowData->Up,
				RowData->Back,
				RowData->Right,
				RowData->Left,
				RowData->Front,
				RowData->Down,
				RowData->Weight
			);
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("Failed to cast BaseTileInfo RowData"));
			return false;
		}
	}

	return true;
}

bool UWFC3DModelDataAsset::InitializeFaceInfo()
{
}

bool UWFC3DModelDataAsset::InitializeTileInfo()
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
	if (TileVariantDataTable == nullptr)
	{
		UE_LOG(LogTemp, Error, TEXT("TileVariantDataTable is nullptr"));
		return false;
	}

	if (BaseTileInfos.IsEmpty())
	{
		UE_LOG(LogTemp, Error, TEXT("BaseTileInfo is Empty"));
		return false;
	}
	
	TileVariants.Empty();
	TileVariants.SetNum(BaseTileInfos.Num());
	
	const TMap<FName, uint8*>& RowMap = TileVariantDataTable->GetRowMap();

	// RowMap 순회
	for (const TPair<FName, uint8*>& Row : RowMap)
	{
		if (const FTileVariantInfoTable* RowData = reinterpret_cast<const FTileVariantInfoTable*>(Row.Value))
		{
			FTileVariantInfo& TileVariantInfo = TileVariants[BaseTileNameToIndex[RowData->TileName]];
			FTileByBiome& BiomeInfo = TileVariantInfo.Biomes.FindOrAdd(RowData->BiomeName);
			BiomeInfo.Tiles.Emplace(RowData->TileMesh, RowData->Materials, RowData->Weight);
			BiomeInfo.TotalWeight += RowData->Weight;
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("Failed to cast TileVariantInfo RowData"));
			return false;
		}
	}
	return true;
}

bool UWFC3DModelDataAsset::InitializeTileRotationInfo()
{
	
}
