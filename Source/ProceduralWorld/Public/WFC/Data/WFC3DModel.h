// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Containers/BitArray.h"
#include "Engine/DataAsset.h"
#include "WFC3DModel.generated.h"

class UStaticMesh;
class UMaterialInterface;

/**
 * 하나의 타일 정보
 */
USTRUCT(BlueprintType)
struct FTile
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "WFC3D|Data")
	UStaticMesh* StaticMesh = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "WFC3D|Data")
	TArray<UMaterialInterface*> Materials = TArray<UMaterialInterface*>();

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "WFC3D|Data")
	float Weight = 1.0f;
};

/*
 * 바이옴 별 타일 정보
 */
USTRUCT(BlueprintType)
struct FTileByBiome
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "WFC3D|Data")
	TArray<FTile> Tiles;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "WFC3D|Data")
	float TotalWeight = 0.0f;

	void CalculateTotalWeight();
};

/**
 * 한 종류의 타일에 대한 타일 바리에이션 정보
 */

USTRUCT(BlueprintType)
struct FTileVariantInfo
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "WFC3D|Data")
	TMap<FName, FTileByBiome> Biomes;

	void CalculateTotalWeight();
};

/*
 * 한 종류의 타일에 대한 타일 바리에이션 정보 테이블
 */

USTRUCT()
struct FTileVariantInfoDataTable : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "WFC3D|Data")
	FName TileName = "";

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "WFC3D|Data")
	FName BiomeName = "";

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "WFC3D|Data")
	UStaticMesh* TileMesh;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "WFC3D|Data")
	TArray<UMaterialInterface*> Materials;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "WFC3D|Data")
	float Weight = 1.0f;
};


/**
 * 데이터 에셋에 보여질 데이터 구조체
 * 베이스 타일 데이터 에셋
 *	- 조각에 대한 가중치
 *  - 각 면에 대한 면 이름 (UBRLFD 순서대로)
 */

USTRUCT(BlueprintType)
struct FBaseTileInfo
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "WFC3D|Data")
	float TileWeight = 1.0f;

	FTileVariantInfo* TileVariations;

	// Faces Size is Fixed to 6
	UPROPERTY(EditAnywhere, BlueprintReadOnly, EditFixedSize, Category = "WFC3D|Data",
		DisplayName = "Faces (Up, Back, Right, Left, Front, Down)")
	TArray<FString> Faces = {"", "", "", "", "", ""};

	bool operator==(const FBaseTileInfo& Other) const
	{
		return TileWeight == Other.TileWeight && TileVariations == Other.TileVariations && Faces == Other.Faces;
	}

	bool operator!=(const FBaseTileInfo& Other) const
	{
		return TileWeight != Other.TileWeight || TileVariations != Other.TileVariations || Faces != Other.Faces;
	}
};

/*
 * 기본 타일 정보 데이터 테이블
 * 타일 정보를 작성하기 쉽게 CSV 파일로 읽어오기 위한 테이블
 */

USTRUCT()
struct FBaseTileInfoDataTable : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "WFC3D|Data")
	float TileWeight = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "WFC3D|Data")
	FString Up = "";
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "WFC3D|Data")
	FString Back = "";

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "WFC3D|Data")
	FString Right = "";

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "WFC3D|Data")
	FString Left = "";

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "WFC3D|Data")
	FString Front = "";
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "WFC3D|Data")
	FString Down = "";
};

/*
 * FacePair 저장용 자료구조
 */

USTRUCT(BlueprintType)
struct FFacePair
{
	GENERATED_BODY()

private:
	UPROPERTY(VisibleAnywhere, Category = "WFC3D|Data")
	int32 Direction = -1;

	UPROPERTY(VisibleAnywhere, Category = "WFC3D|Data")
	FString Name;

public:
	FFacePair()
	{
	}

	FFacePair(int32 InOrder, const FString& InName) : Direction(InOrder), Name(InName)
	{
	}

	TPair<int32, FString> GetPair() const
	{
		return TPair<int32, FString>(Direction, Name);
	}

	int32 GetDirection() const
	{
		return Direction;
	}

	FString GetName() const
	{
		return Name;
	}

	bool operator==(const FFacePair& Other) const
	{
		return Direction == Other.Direction && Name == Other.Name;
	}

	bool operator!=(const FFacePair& Other) const
	{
		return !(*this == Other);
	}

	friend uint32 GetTypeHash(const FFacePair& FacePair)
	{
		return HashCombine(GetTypeHash(FacePair.Direction), GetTypeHash(FacePair.Name));
	}
};

/*
 * BitArray 저장용 자료구조
 */

USTRUCT(BlueprintType)
struct FTileBitString
{
	GENERATED_BODY()

private:
	UPROPERTY(VisibleAnywhere, Category = "WFC3D|Data")
	FString BitString;

public:
	FTileBitString()
	{
	}

	FTileBitString(const TBitArray<>& InBitArray)
	{
		BitString = ToString(InBitArray);
	}

	// 비트셋 가져오기
	TBitArray<> GetBitArray() const
	{
		return ToBitArray(BitString);
	}

	// 비트 수 가져오기
	int32 Num() const
	{
		return BitString.Len();
	}

	static FString ToString(const TBitArray<>& BitArray)
	{
		FString BitString;
		for (int32 i = 0; i < BitArray.Num(); i++)
		{
			BitString += BitArray[i] ? TEXT("1") : TEXT("0");
		}
		return BitString;
	}

	static TBitArray<> ToBitArray(const FString& BitString)
	{
		TBitArray<> BitArray;
		BitArray.Init(false, BitString.Len());
		for (int32 i = 0; i < BitString.Len(); i++)
		{
			BitArray[i] = BitString[i] == '1';
		}
		return BitArray;
	}
};

/*
 * WFC 3D 모델 데이터 에셋
 */
UCLASS()
class PROCEDURALWORLD_API UWFC3DModel : public UDataAsset
{
	GENERATED_BODY()

	UWFC3DModel();

public:
	// 직접 입력해야하는 정보
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "WFC3D|Data")
	TObjectPtr<UDataTable> BaseTileDataTable;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "WFC3D|Data")
	TObjectPtr<UDataTable> TileVariantDataTable;


	// 자동으로 설정되는 정보
	// 베이스 타일 정보
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "WFC3D|Data")
	TMap<FName, FBaseTileInfo> BaseTileInfos;

	// 타일 바리에이션 정보
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "WFC3D|Data")
	TMap<FName, FTileVariantInfo> TileVariantInfos;

	// 모든 타일 정보
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "WFC3D|Data")
	TArray<FBaseTileInfo> TileInfos;

	// 모든 면 정보: 면 위치(UBRLFD순서), 면 이름
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "WFC3D|Data")
	TArray<FFacePair> FaceInfos;

	
	// TODO: FaceInfo에서는 Direction과 관계없이 모든 방향이 들어있다. 이걸 각 방향에 대한 정보로 나눠서 저장해야함
	// FacePair 대신에 FaceArray를 만들고, FaceArray를 6개 가진 FaceInfo가 있음
	// FaceInfo에서 각 FaceArray에는 각 방향에 대해서 FaceString이 들어가있음
	// Ex) FaceArray[0] = Up 방향에 가능한 모든 면 이름 {'1a', '1b', '1c', '1d', '2a', '2b', '2c', '2d', ...}
	// 이걸 근데 어떻게 TileToFace로 바꾸지?
	// TileToFace는 각 타일에 대해서 TArray<TBitArray<>>로 저장해야하고 각 TBitArray<>에는 각 방향에 대한 면 인덱스가 들어가야함
	// Ex) TileToFace[0] = {0000000010000, 00001000000, 00000100000, 00000100000, 00001000000, 0000000000010000}
	
	
	// 면 인덱스 -> 타일 맵 ((면 위치(UBRLFD순서)) -> Bitset)
	// 해당 면이 셀에 위치할 때 해당 면과 맞닿을 수 있는 모든 조각에 대한 인덱스에 대한 비트셋
	// Ex) 0 -> FaceToTileBitMapKeys에서 인덱스가 0번인 면에 대해서 반대편에 맞는 면을 가진 조각을 TileInfos에서 찾아서 그 인덱스를 비트셋에 추가
	// TBitArray의 크기는 TileInfos의 사이즈와 같다.
	TMap<int32, TBitArray<>> FaceToTileBitArrayMap;

	// FaceToTileBitArrayMap를 데이터 에셋에 저장하기 위한 자료구조
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "WFC3D|Data")
	TMap<int32, FTileBitString> FaceToTileBitStringMap;

	// 타일 인덱스 -> 면 맵 (TileInfosIndex -> Bitset)
	// 해당 조각이 셀에 위치할 때 해당 조각이 가지는 모든 면에 대한 인덱스에 대한 비트셋
	// Ex) 0 -> 0번 조각이 가지는 모든 면(6개)에 대한 인덱스를 비트셋에 추가
	// TBitArray의 크기는 FaceInfos의 사이즈와 같다.
	TMap<int32, TBitArray<>> TileToFaceBitArrayMap;

	// TileToFaceBitArrayMap를 데이터 에셋에 저장하기 위한 자료구조
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "WFC3D|Data")
	TMap<int32, FTileBitString> TileToFaceBitStringMap;

private:
	// 베이스 타일 정보 설정
	bool SetBaseTileInfos();

	// 타일 바리에이션 정보 설정
	bool SetTileVariantInfos();

	// BaseTileInfo의 FTileVariantInfoSet 설정
	bool SetBaseTileVariantInfoSet();

	// TileInfos 설정
	// BaseTileInfo를 회전 시켜서 나올 수 있는 모든 가능성을 추가
	bool SetTileInfos();

	// 면 비트맵을 설정하기 위한 키 설정
	bool SetFaceToTileBitMapKeys();

	// 면 비트맵 설정
	bool SetFaceToTileBitStringMap();

	// 면 비트맵 로드
	bool LoadFaceToTileBitMap();

	// 타일 비트맵 설정
	bool SetTileToFaceBitStringMap();

	// 타일 비트맵 로드
	bool LoadTileToFaceBitMap();

public:
	// 데이터 생성
	bool CreateData();

	// 데이터 로드
	bool LoadData();

	// 데이터 출력
	bool PrintData();

private:
	// Debug
	void PrintFaceToTileBitMapKeys()
	{
		UE_LOG(LogTemp, Display, TEXT("FaceToTileBitMapKeys Size: %d"), FaceInfos.Num());
		for (int32 i = 0; i < FaceInfos.Num(); i++)
		{
			UE_LOG(LogTemp, Display, TEXT("FaceToTileBitMapKey %d: (%d, %s)"), i, FaceInfos[i].GetPair().Key, *FaceInfos[i].GetPair().Value);
		}
	}

	void PrintFaceToTileBitMap()
	{
		UE_LOG(LogTemp, Display, TEXT("FaceToTileBitMap Size: %d"), FaceToTileBitArrayMap.Num());
		for (auto& Elem : FaceToTileBitArrayMap)
		{
			UE_LOG(LogTemp, Display, TEXT("FaceToTileBitMap Key: %d"), Elem.Key);
			UE_LOG(LogTemp, Display, TEXT("FaceToTileBitMap Value: %s"), *FTileBitString::ToString(Elem.Value));
		}
	}

	void PrintTileToFaceBitMap()
	{
		UE_LOG(LogTemp, Display, TEXT("TileToFaceBitMap Size: %d"), TileToFaceBitArrayMap.Num());
		for (auto& Elem : TileToFaceBitArrayMap)
		{
			UE_LOG(LogTemp, Display, TEXT("TileToFaceBitMap Key: %d"), Elem.Key);
			UE_LOG(LogTemp, Display, TEXT("TileToFaceBitMap Value: %s"), *FTileBitString::ToString(Elem.Value));
		}
	}

	void PrintTileVariationInfo()
	{
		UE_LOG(LogTemp, Display, TEXT("TileVariationInfo Size: %d"), TileVariantInfos.Num());
		for (auto& Elem : TileVariantInfos)
		{
			UE_LOG(LogTemp, Display, TEXT("TileName: %s"), *Elem.Key.ToString());
			for (auto& Biome : Elem.Value.Biomes)
			{
				UE_LOG(LogTemp, Display, TEXT("    Biome: %s"), *Biome.Key.ToString());
				for (auto& Tile : Biome.Value.Tiles)
				{
					UE_LOG(LogTemp, Display, TEXT("        Tile StaticMesh: %s"), *Tile.StaticMesh->GetName());
					UE_LOG(LogTemp, Display, TEXT("        Tile Materials Size: %d"), Tile.Materials.Num());
					for (auto& Material : Tile.Materials)
					{
						UE_LOG(LogTemp, Display, TEXT("            Tile Material: %s"), *Material->GetName());
					}
					UE_LOG(LogTemp, Display, TEXT("        Tile Weight: %f"), Tile.Weight);
				}
				UE_LOG(LogTemp, Display, TEXT("    Biome TotalWeight: %f"), Biome.Value.TotalWeight);
			}
		}
	}
	
	const int32 RotationMap[4][4] = {
		{1, 4, 3, 2},
		{2, 1, 4, 3},
		{4, 3, 2, 1},
		{3, 2, 1, 4}
	};

public:
	bool HasMatchingFace(const FFacePair& Face, const TArray<FString>& Faces);
	FString RotateUDFace(const FString& InputString, const int32& RotationSteps);
	FBaseTileInfo RotateTileClockwise(const FBaseTileInfo& BaseTileInfo, const int32& RotationStep);


	// WFC3DActor에 필요함
	// TArray<int32> GetAllIndices() const
	// {
	// 	TArray<int32> Result;
	// 	int32 Index = 0;
	// 	do
	// 	{
	// 		Index = BitArray.FindFrom(true, Index);
	// 		if (Index != INDEX_NONE)
	// 		{
	// 			Result.Add(Index++);
	// 		}
	// 	}
	// 	while (Index != INDEX_NONE);
	// 	return Result;
	// }

	// FTileBitString operator&(const FTileBitString& Rhs) const
	// {
	// 	return FTileBitString(TBitArray<>::BitwiseAND(BitArray, Rhs.BitArray, EBitwiseOperatorFlags::MaxSize));
	// }
};
