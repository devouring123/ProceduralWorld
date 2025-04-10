// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Containers/BitArray.h"
#include "Engine/DataAsset.h"
#include "WFC3DModel.generated.h"

class UStaticMesh;
class UMaterialInterface;

UENUM(BlueprintType)
enum class EFace : uint8
{
	Up = 0 UMETA(DisplayName = "Up"),
	Back = 1 UMETA(DisplayName = "Back"),
	Right = 2 UMETA(DisplayName = "Right"),
	Left = 3 UMETA(DisplayName = "Left"),
	Front = 4 UMETA(DisplayName = "Front"),
	Down = 5 UMETA(DisplayName = "Down"),
	None = 6 UMETA(Hidden),
};

/**
 * FaceUtils
 */
class FFaceUtils
{
public:
	static constexpr EFace AllDirections[6] = {
		EFace::Up,
		EFace::Back,
		EFace::Right,
		EFace::Left,
		EFace::Front,
		EFace::Down
	};

	static FORCEINLINE EFace GetOpposite(EFace Face)
	{
		const uint8 Index = static_cast<uint8>(Face);
		return Index < 6 ? static_cast<EFace>(5 - Index) : EFace::None;
	}

	static FORCEINLINE uint8 ToIndex(const EFace& State)
	{
		return static_cast<uint8>(State);
	}

	static FORCEINLINE uint8 ToOppositeIndex(const EFace& State)
	{
		return 5 - static_cast<uint8>(State);
	}

	static FORCEINLINE FIntVector GetDirectionVector(EFace Face)
	{
		const uint8 Index = static_cast<uint8>(Face);
		return Index < 6 ? DirectionVectors[Index] : FIntVector(0, 0, 0);
	}

	static FORCEINLINE EFace Rotate(const EFace& Face, const uint8& Step)
	{
		const uint8 Index = static_cast<uint8>(Face);
		return RotationMap[Index][Step];
	}

private:
	static const FIntVector DirectionVectors[6];

	static constexpr EFace RotationMap[6][4] = {
		{EFace::None, EFace::None, EFace::None, EFace::None},
		{EFace::Back, EFace::Left, EFace::Front, EFace::Right},
		{EFace::Right, EFace::Back, EFace::Left, EFace::Front},
		{EFace::Left, EFace::Front, EFace::Right, EFace::Back},
		{EFace::Front, EFace::Right, EFace::Back, EFace::Left},
		{EFace::None, EFace::None, EFace::None, EFace::None},
	};
};


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
		return !(*this == Other);
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
	EFace Direction = EFace::None;

	UPROPERTY(VisibleAnywhere, Category = "WFC3D|Data")
	FString Name;

public:
	FFacePair()
	{
	}

	FFacePair(EFace InDirection, const FString& InName) : Direction(InDirection), Name(InName)
	{
	}

	TPair<EFace, FString> GetPair() const
	{
		return TPair<EFace, FString>(Direction, Name);
	}

	EFace GetDirection() const
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
		for (uint8 i = 0; i < BitArray.Num(); ++i)
		{
			BitString += BitArray[i] ? TEXT("1") : TEXT("0");
		}
		return BitString;
	}

	static TBitArray<> ToBitArray(const FString& BitString)
	{
		TBitArray<> BitArray;
		BitArray.Init(false, BitString.Len());
		for (uint8 i = 0; i < BitString.Len(); ++i)
		{
			BitArray[i] = BitString[i] == '1';
		}
		return BitArray;
	}
};

/**
 * 각 타일의 면 정보
 */
USTRUCT(BlueprintType)
struct FTileFaceIndices
{
	GENERATED_BODY()

	FTileFaceIndices() = default;

	FTileFaceIndices(TArray<int32>&& InFaceIndices) : FaceIndices(InFaceIndices)
	{
	}

	// Faces Size is Fixed to 6
	UPROPERTY(EditAnywhere, BlueprintReadOnly, EditFixedSize, Category = "WFC3D|Data",
		DisplayName = "FaceIndices (Up, Back, Right, Left, Front, Down)")
	TArray<int32> FaceIndices = {0, 0, 0, 0, 0, 0};
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

	
	// FacePair 대신에 FaceArray를 만들고, FaceArray를 6개 가진 FaceInfo가 있음
	// FaceInfo에서 각 FaceArray에는 각 방향에 대해서 FaceString이 들어가있음
	// Ex) FaceArray[0] = Up 방향에 가능한 모든 면 이름 {'1a', '1b', '1c', '1d', '2a', '2b', '2c', '2d', ...}
	// 이걸 근데 어떻게 TileToFace로 바꾸지?
	// TileToFace는 각 타일에 대해서 TArray<TBitArray<>>로 저장해야하고 각 TBitArray<>에는 각 방향에 대한 면 인덱스가 들어가야함
	// Ex) TileToFace[0] = {000000001000000, 00001000000, 00001000000, 00001000000, 00001000000, 000000000010000}
	// TSet을 사용해서 전부 합치는 연산을 하기 -> BitArray로 바로 변환하면 연산 안해도 됨

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
	UPROPERTY(EditAnywhere, Blueprintable, Category = "WFC3D|Data")
	TMap<int32, FTileFaceIndices> TileToFaceMap;

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
	bool SetTileToFaceMap();

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
		for (uint8 i = 0; i < FaceInfos.Num(); ++i)
		{
			UE_LOG(LogTemp, Display, TEXT("FaceToTileBitMapKey %d: (%d, %s)"), i, FaceInfos[i].GetPair().Key,
			       *FaceInfos[i].GetPair().Value);
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

public:
	static bool HasMatchingFace(const FFacePair& Face, const TArray<FString>& Faces);
	static FString RotateUDFace(const FString& InputString, const int32& RotationSteps);
	static FBaseTileInfo RotateTileClockwise(const FBaseTileInfo& BaseTileInfo, const int32& RotationStep);
};
