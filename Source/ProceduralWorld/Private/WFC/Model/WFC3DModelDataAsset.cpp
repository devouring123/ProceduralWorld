// Fill out your copyright notice in the Description page of Project Settings.


#include "WFC/Model/WFC3DModelDataAsset.h"
#include "WFC/Model/WFC3DFaceUtils.h"


bool UWFC3DModelDataAsset::InitializeData()
{
	bool bSuccess = true;
	bSuccess &= InitializeCommonData();
	bSuccess &= InitializeAlgorithmData();
	bSuccess &= InitializeVisualizationData();
	return bSuccess;
}

bool UWFC3DModelDataAsset::InitializeCommonData()
{
	bool bSuccess = true;
	bSuccess &= InitializeBaseTileInfo();
	bSuccess &= InitializeFaceInfo();
	bSuccess &= InitializeTileInfo();
	return bSuccess;
}

bool UWFC3DModelDataAsset::InitializeAlgorithmData()
{
	bool bSuccess = true;
	bSuccess &= InitializeFaceToTileBitStringAndBitArray();
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

const TBitArray<>* UWFC3DModelDataAsset::GetCompatibleTiles(const int32 FaceIndex) const
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

const TArray<int32>* UWFC3DModelDataAsset::GetTileFaceIndices(const int32 TileIndex) const
{
	if (TileInfos.IsEmpty())
	{
		UE_LOG(LogTemp, Error, TEXT("TileInfos is Empty"));
		return nullptr;
	}
	if (TileIndex < 0 || TileIndex >= TileInfos.Num())
	{
		UE_LOG(LogTemp, Error, TEXT("TileIndex is out of range"));
		return nullptr;
	}
	return &TileInfos[TileIndex].Faces;
}

const float UWFC3DModelDataAsset::GetTileWeight(const int32 TileIndex) const
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

const FTileVariantInfo* UWFC3DModelDataAsset::GetTileVariant(const int32 TileIndex) const
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

/** TODO: 구현 */
const FTileVisualInfo* UWFC3DModelDataAsset::GetRandomTileVisualInfo(const int32 BaseTileIndex, const FString& BiomeName) const
{
	return nullptr;
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
	if (BaseTileInfos.IsEmpty())
	{
		UE_LOG(LogTemp, Error, TEXT("BaseTileInfos is Empty"));
		return false;
	}

	FaceInfos.Empty();

	for (const FBaseTileInfo& BaseTileInfo : BaseTileInfos)
	{
		for (const EFace& Direction : FWFC3DFaceUtils::AllDirections)
		{
			/** Up, Down 이면 회전한 면도 추가해야함 */
			if (Direction == EFace::Up || Direction == EFace::Down)
			{
				FaceInfos.AddUnique(FFaceInfo(Direction, BaseTileInfo.Faces[FWFC3DFaceUtils::GetIndex(Direction)]));
				for (int32 RotationStep = 1; RotationStep < 4; ++RotationStep)
				{
					FaceInfos.AddUnique(
						FFaceInfo(
							Direction,
							FWFC3DFaceUtils::RotateUDFace(BaseTileInfo.Faces[FWFC3DFaceUtils::GetIndex(Direction)], RotationStep)
						)
					);
				}
			}
			else
			{
				/** BRLF 면은 회전한 면을 추가하지 않음 */
				FaceInfos.AddUnique(FFaceInfo(Direction, BaseTileInfo.Faces[FWFC3DFaceUtils::GetIndex(Direction)]));
			}
		}
	}

	/** 면 -> 인덱스 설정 */
	int FaceInfoSize = FaceInfos.Num();
	for (int32 Index = 0; Index < FaceInfoSize; ++Index)
	{
		FaceToIndex.Add(FaceInfos[Index], Index);
	}

	/** 반대 방향 면 인덱스 설정 */
	for (int32 Index = 0; Index < FaceInfos.Num(); ++Index)
	{
		/** UD(Up/Down) 면은 반대방향으로 설정 */
		if (FaceInfos[Index].Direction == EFace::Up || FaceInfos[Index].Direction == EFace::Down)
		{
			OppositeFaceIndex[Index] = FaceToIndex.FindChecked(
				FFaceInfo(
					FWFC3DFaceUtils::GetOpposite(FaceInfos[Index].Direction),
					FaceInfos[Index].Name
				)
			);
		}
		else
		{
			OppositeFaceIndex[Index] = FaceToIndex.FindChecked(
				FFaceInfo(
					FWFC3DFaceUtils::GetOpposite(FaceInfos[Index].Direction),
					FWFC3DFaceUtils::FlipBRLFFace(FaceInfos[Index].Name)
				)
			);
		}
	}

	return true;
}

bool UWFC3DModelDataAsset::InitializeTileInfo()
{
	if (BaseTileInfos.IsEmpty())
	{
		UE_LOG(LogTemp, Error, TEXT("BaseTileInfos is Empty"));
		return false;
	}

	TileInfos.Empty();

	for (int32 Index = 0; Index < BaseTileInfos.Num(); ++Index)
	{
		FTileInfo TileInfo;
		TileInfo.BaseTileID = Index;
		for (const EFace& Direction : FWFC3DFaceUtils::AllDirections)
		{
			TileInfo.Faces[FWFC3DFaceUtils::GetIndex(Direction)] =
				FaceToIndex.FindChecked({Direction, BaseTileInfos[Index].Faces[FWFC3DFaceUtils::GetIndex(Direction)]});
		}
		TileInfo.Weight = BaseTileInfos[Index].Weight;
		TileInfos.AddUnique(TileInfo);
		for (uint8 RotationStep = 1; RotationStep < 4; ++RotationStep)
		{
			TileInfos.AddUnique(RotateTileClockwise(TileInfo, RotationStep));
		}
	}
	return true;
}

bool UWFC3DModelDataAsset::InitializeFaceToTileBitStringAndBitArray()
{
	if (FaceInfos.IsEmpty())
	{
		UE_LOG(LogTemp, Error, TEXT("FaceInfos is Empty"));
		return false;
	}
	if (TileInfos.IsEmpty())
	{
		UE_LOG(LogTemp, Error, TEXT("TileInfos is Empty"));
		return false;
	}
	
	FaceToTileBitArrays.Empty();
	FaceToTileBitStrings.Empty();
	
	int32 TileSetSize = TileInfos.Num();
	for (const FFaceInfo& Face : FaceInfos)
	{
		TBitArray<> NewBitArray;
		NewBitArray.Init(false, TileSetSize);
		int32 OppositeFaceIndex = FaceToIndex[FWFC3DFaceUtils::GetOppositeFace(Face)];
		for (int32 TileIndex = 0; TileIndex < TileSetSize; ++TileIndex)
		{
			if (OppositeFaceIndex == TileInfos[TileIndex].Faces[FWFC3DFaceUtils::GetOppositeIndex(Face.Direction)])
			{
				NewBitArray[TileIndex] = true;
			}
		}
		FaceToTileBitArrays.Add(NewBitArray);
		FaceToTileBitStrings.Add(MoveTemp(NewBitArray));
	}
	return true;
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

/** TODO: 구현 */
bool UWFC3DModelDataAsset::InitializeTileRotationInfo()
{
	return true;
}

FTileInfo UWFC3DModelDataAsset::RotateTileClockwise(const FTileInfo& TileToRotate, const int32 RotationStep)
{
	FTileInfo NewTileInfo(TileToRotate);
	// UD(Up/Down) 회전
	NewTileInfo.Faces[FWFC3DFaceUtils::GetIndex(EFace::Up)] =
		FaceToIndex[FFaceInfo(
			EFace::Up,
			FWFC3DFaceUtils::RotateUDFace(
				FaceInfos[TileToRotate.Faces[FWFC3DFaceUtils::GetIndex(EFace::Up)]].Name,
				RotationStep
			)
		)];
	NewTileInfo.Faces[FWFC3DFaceUtils::GetIndex(EFace::Down)] =
		FaceToIndex[FFaceInfo(
			EFace::Down,
			FWFC3DFaceUtils::RotateUDFace(
				FaceInfos[TileToRotate.Faces[FWFC3DFaceUtils::GetIndex(EFace::Down)]].Name,
				RotationStep
			)
		)];

	// BRLF(Back/Right/Left/Front) 회전
	for (const EFace& SideDirection : {EFace::Back, EFace::Right, EFace::Left, EFace::Front})
	{
		NewTileInfo.Faces[FWFC3DFaceUtils::GetIndex(SideDirection)] =
			TileToRotate.Faces[FWFC3DFaceUtils::GetIndex(FWFC3DFaceUtils::Rotate(SideDirection, RotationStep))];
	}

	return MoveTemp(NewTileInfo);
}
