// Fill out your copyright notice in the Description page of Project Settings.


#include "WFC/Data/WFC3DModel.h"


void FTileByBiome::CalculateTotalWeight()
{
	// Calculate the total weight of all variants
	TotalWeight = 0.0f;
	for (const FTile& Tile : Tiles)
	{
		TotalWeight += Tile.Weight;
	}
}

void FTileVariantInfo::CalculateTotalWeight()
{
	// Calculate the total weight of all variants
	for (TTuple<FName, FTileByBiome>& Biome : Biomes)
	{
		Biome.Value.CalculateTotalWeight();
	}
}


const EFace UWFC3DModel::RotationMap[6][4] = {
	{EFace::None, EFace::None, EFace::None, EFace::None},
	{EFace::Back, EFace::Left, EFace::Front, EFace::Right},
	{EFace::Right, EFace::Back, EFace::Left, EFace::Front},
	{EFace::Left, EFace::Front, EFace::Right, EFace::Back},
	{EFace::Front, EFace::Right, EFace::Back, EFace::Left},
	{EFace::None, EFace::None, EFace::None, EFace::None},
};


UWFC3DModel::UWFC3DModel()
{
}


bool UWFC3DModel::SetBaseTileInfos()
{
	if (BaseTileDataTable == nullptr)
	{
		UE_LOG(LogTemp, Error, TEXT("BaseTileDataTable is nullptr"));
		return false;
	}

	BaseTileInfos.Empty();
	const TMap<FName, uint8*>& RowMap = BaseTileDataTable->GetRowMap();

	// RowMap 순회
	for (const TPair<FName, uint8*>& Row : RowMap)
	{
		if (const FBaseTileInfoDataTable* RowData = reinterpret_cast<const FBaseTileInfoDataTable*>(Row.Value))
		{
			FBaseTileInfo NewBaseTileInfo;
			NewBaseTileInfo.Faces = {
				RowData->Up,
				RowData->Back,
				RowData->Right,
				RowData->Left,
				RowData->Front,
				RowData->Down,
			};
			NewBaseTileInfo.TileWeight = RowData->TileWeight;
			BaseTileInfos.Add(Row.Key, MoveTemp(NewBaseTileInfo));
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("Failed to cast BaseTileInfo RowData"));
			return false;
		}
	}

	return true;
}

bool UWFC3DModel::SetTileVariantInfos()
{
	if (TileVariantDataTable == nullptr)
	{
		UE_LOG(LogTemp, Error, TEXT("TileVariantDataTable is nullptr"));
		return false;
	}

	TileVariantInfos.Empty();
	const TMap<FName, uint8*>& RowMap = TileVariantDataTable->GetRowMap();

	// RowMap 순회
	for (const TPair<FName, uint8*>& Row : RowMap)
	{
		if (const FTileVariantInfoDataTable* RowData = reinterpret_cast<const FTileVariantInfoDataTable*>(Row.Value))
		{
			// 타일 변형 정보 찾고, 타일 변형 정보가 없으면 생성
			// 바이옴도 찾고 바이옴이 없으면 생성
			FTileVariantInfo& TileVariantInfo = TileVariantInfos.FindOrAdd(RowData->TileName);
			FTileByBiome& BiomeInfo = TileVariantInfo.Biomes.FindOrAdd(RowData->BiomeName);
			// 타일 생성
			FTile NewTile;
			NewTile.StaticMesh = RowData->TileMesh;
			NewTile.Materials = RowData->Materials;
			NewTile.Weight = RowData->Weight;
			// 타일 추가
			BiomeInfo.Tiles.Add(MoveTemp(NewTile));
			BiomeInfo.TotalWeight += NewTile.Weight;
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("Failed to cast TileVariantInfo RowData"));
			return false;
		}
	}
	return true;
}

bool UWFC3DModel::SetBaseTileVariantInfoSet()
{
	if(BaseTileInfos.IsEmpty())
	{
		UE_LOG(LogTemp, Error, TEXT("BaseTileInfos is Empty"));
		return false;
	}
	if(TileVariantInfos.IsEmpty())
    {
        UE_LOG(LogTemp, Error, TEXT("TileVariantInfos is Empty"));
        return false;
    }
	for (auto& BaseTileInfo : BaseTileInfos)
	{
		if(TileVariantInfos.Find(BaseTileInfo.Key) == nullptr)
        {
            UE_LOG(LogTemp, Error, TEXT("TileVariantInfos does not have BaseTileInfo by Key: %s"), *BaseTileInfo.Key.ToString());
            // return false;
        }
		BaseTileInfo.Value.TileVariations = TileVariantInfos.Find(BaseTileInfo.Key);
	}
	return true;
}

bool UWFC3DModel::SetTileInfos()
{
	TileInfos.Empty();
	if(BaseTileInfos.IsEmpty())
	{
		UE_LOG(LogTemp, Error, TEXT("BaseTileInfos is Empty"));
		return false;
	}
	for (const auto& BaseTileInfo : BaseTileInfos)
	{
		TileInfos.AddUnique(BaseTileInfo.Value);
		for (int32 RotationStep = 1; RotationStep < 4; ++RotationStep)
		{
			TileInfos.AddUnique(RotateTileClockwise(BaseTileInfo.Value, RotationStep));
		}
	}
	return true;
}

bool UWFC3DModel::SetFaceToTileBitMapKeys()
{
	FaceInfos.Empty();
	if(TileInfos.IsEmpty())
	{
		UE_LOG(LogTemp, Error, TEXT("TileInfos is Empty"));
		return false;
	}
	for (const auto& TileInfo : TileInfos)
	{
		for (int32 Direction = 0; Direction < 6; ++Direction)
		{
			FaceInfos.AddUnique({static_cast<EFace>(Direction), TileInfo.Faces[Direction]});
		}
	}
	return true;
}

bool UWFC3DModel::SetFaceToTileBitStringMap()
{
	FaceToTileBitStringMap.Empty();
	int32 TileSetSize = TileInfos.Num();
	int32 FaceToTileBitMapKeysSize = FaceInfos.Num();

	for(int32 i = 0; i < FaceToTileBitMapKeysSize; ++i)
	{
		FFacePair Face = FaceInfos[i];
		TBitArray<> NewBitArray;
		NewBitArray.Init(false, TileSetSize);

		for (int32 j = 0; j < TileSetSize; ++j)
		{
			if (HasMatchingFace(Face, TileInfos[j].Faces))
			{
				NewBitArray[j] = true;
			}
		}

		FaceToTileBitStringMap.Add(i, MoveTemp(NewBitArray));
	}
	return true;
}

bool UWFC3DModel::SetTileToFaceMap()
{
	TileToFaceMap.Empty();
	int32 TileSetSize = TileInfos.Num();

	// TileToFaceMap에 각 타일이 가지는 모든 면에 대한 인덱스 추가
	for (int32 i = 0; i < TileSetSize; ++i)
	{
		TArray<int32> FaceIndices;
		// TODO: EFace의 모든 방향 확인
		for (int32 Direction = 0; Direction < 6; ++Direction)
		{
			FaceIndices.Add(FaceInfos.Find({static_cast<EFace>(Direction), TileInfos[i].Faces[Direction]}));
		}

		TileToFaceMap.Add(i, MoveTemp(FaceIndices));
	}
	return true;
}

bool UWFC3DModel::LoadFaceToTileBitMap()
{
	if (FaceToTileBitStringMap.Num() == 0)
	{
		UE_LOG(LogTemp, Error, TEXT("FaceToTileBitStringMap is Empty"));
		return false;
	}

	for (const auto& Tuple : FaceToTileBitStringMap)
	{
		FaceToTileBitArrayMap.Add(Tuple.Key, Tuple.Value.GetBitArray());
	}
	return true;
}

bool UWFC3DModel::CreateData()
{
	Modify();

	if (!SetBaseTileInfos())
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to SetBaseTileInfos"));
		return false;
	}

	if (!SetTileVariantInfos())
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to SetTileVariantInfos"));
		return false;
	}

	if(!SetBaseTileVariantInfoSet())
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to SetBaseTileVariantInfoSet"));
        return false;
    }
	
	if (!SetTileInfos())
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to SetTileInfos"));
		return false;
	}

	if (!SetBaseTileVariantInfoSet())
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to SetBaseTileVariantInfoSet"));
		return false;
	}

	if (!SetFaceToTileBitMapKeys())
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to SetFaceToTileBitMapKeys"));
		return false;
	}

	if (!SetFaceToTileBitStringMap())
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to SetFaceToTileBitStringMap"));
		return false;
	}

	if (!SetTileToFaceMap())
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to SetTileToFaceBitStringMap"));
		return false;
	}
	// Load Data
	if (!LoadData())
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to LoadData"));
        return false;
    }
	
	// Print Model Data
	if (!PrintData())
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to PrintData"));
		return false;
	}
	
	return true;
}

bool UWFC3DModel::LoadData()
{
	if (!LoadFaceToTileBitMap())
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to LoadFaceToTileBitMap in LoadData"));
		return false;
	}
	
	return true;
}

bool UWFC3DModel::PrintData()
{
	if(!TileVariantInfos.IsEmpty())
	{
		PrintTileVariationInfo();
	}
	
	PrintFaceToTileBitMapKeys();

	if (!FaceToTileBitArrayMap.IsEmpty())
	{
		PrintFaceToTileBitMap();
	}
	else
	{
		UE_LOG(LogTemp, Display, TEXT("Failed to PrintFaceToTileBitMap in PrintData"));
		if (!LoadFaceToTileBitMap())
        {
            UE_LOG(LogTemp, Error, TEXT("Failed to LoadFaceToTileBitMap in PrintData"));
            return false;
        }
	}

	return true;
}

bool UWFC3DModel::HasMatchingFace(const FFacePair& FacePair, const TArray<FString>& Faces)
{
	// Compare UD Face
	TPair<EFace, FString> Face = FacePair.GetPair();
	if ((Face.Key == EFace::Up || Face.Key == EFace::Down) && Faces[ToOppositeIndex(Face.Key)] == Face.Value)
	{
		return true;
	}
	// Compare BRLF Face
	if (Face.Key >= EFace::Back && Face.Key <= EFace::Left)
	{
		// Face1 == "3s", Face2 == "3s" return True
		// Face1 == "2f", Face2 == "3s" return false
		// Face1 == "3s", Face2 == "3" return false

		// TODO: UBRLFD 순서대로 잘 맞게 해줘야 제대로 작동함 
		if (Faces[ToOppositeIndex(Face.Key)] == Face.Value && Faces[ToOppositeIndex(Face.Key)].Find("s") && Face.Value.Find("s"))
		{
			return true;
		}

		// Face1 == "2f", Face2 == "2" return true
		// Face1 == "2", Face2 == "2f" return true
		// Face1 == "2f", Face2 == "2f" return false
		// Face1 == "2", Face2 == "2" return false

		if (Faces[ToOppositeIndex(Face.Key)] == Face.Value + "f" || Faces[ToOppositeIndex(Face.Key)] + "f" == Face.Value)
		{
			return true;
		}
	}
	return false;
}

FString UWFC3DModel::RotateUDFace(const FString& InputString, const int32& RotationSteps)
{
	FRegexPattern Pattern(TEXT("([+-]?\\d+)([a-zA-Z])"));
	FRegexMatcher Matcher(Pattern, InputString);

	if (Matcher.FindNext())
	{
		FString NumberPart = Matcher.GetCaptureGroup(1);
		FString CharPart = Matcher.GetCaptureGroup(2);

		if (CharPart.IsEmpty())
		{
			return InputString;
		}

		// 문자 순환 처리 ('a' -> 'b' -> 'c' -> 'd' -> 'a')
		TCHAR CurrentChar = CharPart[0];
		TCHAR NextChar = (CurrentChar - 'a' + RotationSteps) % 4 + 'a';

		return NumberPart + FString(1, &NextChar);
	}
	return InputString;
}

FBaseTileInfo UWFC3DModel::RotateTileClockwise(const FBaseTileInfo& BaseTileInfo, const int32& RotationStep)
{
	FBaseTileInfo NewTileInfo(BaseTileInfo);

	// UD Rotation

	NewTileInfo.Faces[ToIndex(EFace::Up)] = RotateUDFace(BaseTileInfo.Faces[ToIndex(EFace::Up)], RotationStep);
	NewTileInfo.Faces[ToIndex(EFace::Down)] = RotateUDFace(BaseTileInfo.Faces[ToIndex(EFace::Down)], RotationStep);
	
	// BRLF Rotation
	for (int32 i = 1; i < 5; ++i)
	{
		NewTileInfo.Faces[i] = BaseTileInfo.Faces[ToIndex(RotationMap[i][RotationStep])];
	}

	return NewTileInfo;
}
