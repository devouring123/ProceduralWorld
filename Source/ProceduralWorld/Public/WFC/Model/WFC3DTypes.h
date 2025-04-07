// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Containers/BitArray.h"
#include "Engine/DataTable.h"
#include "WFC3DTypes.generated.h"

class UStaticMesh;
class UMaterialInterface;

/**
 * 면 방향 정의
 */
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
 * 타일 정보 구조체
 * 알고리즘 계산에만 사용함
 */
USTRUCT(BlueprintType)
struct PROCEDURALWORLD_API FTileInfo
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "WFC3D|Data")
	int32 TileID = 0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, EditFixedSize, Category = "WFC3D|Data",
		DisplayName = "Faces (Up, Back, Right, Left, Front, Down)")
	TArray<FString> Faces = {"", "", "", "", "", ""};

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "WFC3D|Data")
	float Weight = 1.0f;
};

/**
 * 면 정보 구조체
 * 알고리즘 계산에만 사용함
 */
USTRUCT(BlueprintType)
struct PROCEDURALWORLD_API FFaceInfo
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "WFC3D|Data")
	EFace Direction = EFace::None;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "WFC3D|Data")
	FString Name;

	FFaceInfo() = default;

	FFaceInfo(const EFace& InDirection, const FString& InName)
		: Direction(InDirection), Name(InName)
	{
	}
};

/**
 * 하나의 타일 정보
 * Visualization에 사용됨
 */
USTRUCT(BlueprintType)
struct PROCEDURALWORLD_API FTile
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "WFC3D|Data")
	UStaticMesh* StaticMesh = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "WFC3D|Data")
	TArray<UMaterialInterface*> Materials = TArray<UMaterialInterface*>();

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "WFC3D|Data")
	float Weight = 1.0f;
};

/**
 * 타일의 면 인덱스 구조체
 * 알고리즘 계산에만 사용함
 */
USTRUCT(BlueprintType)
struct PROCEDURALWORLD_API FTileFaceIndices
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

/**
 * 타일 시각화 정보 구조체
 * Visualization에 사용됨
 */
USTRUCT(BlueprintType)
struct PROCEDURALWORLD_API FTileVisualInfo
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "WFC3D|Data")
	UStaticMesh* Mesh = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "WFC3D|Data")
	TArray<UMaterialInterface*> Materials;

	// 여기서 뽑아서 위를 설정
	FBaseTileInfo* VariantInfo = nullptr;
};

/**
 * 바이옴 별 타일 정보
 * Visualization에 사용됨
 */
USTRUCT(BlueprintType)
struct PROCEDURALWORLD_API FTileByBiome
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
 * Visualization에 사용됨
 */
USTRUCT(BlueprintType)
struct PROCEDURALWORLD_API FTileVariantInfo
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "WFC3D|Data")
	TMap<FName, FTileByBiome> Biomes;

	void CalculateTotalWeight();
};

/**
 * 한 종류의 타일에 대한 타일 바리에이션 정보 테이블
 * Visualization에 사용됨
 */
USTRUCT()
struct PROCEDURALWORLD_API FTileVariantInfoDataTable : public FTableRowBase
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


USTRUCT(BlueprintType)
struct PROCEDURALWORLD_API FBaseTileInfo
{
	GENERATED_BODY()

	// Faces Size is Fixed to 6
	UPROPERTY(EditAnywhere, BlueprintReadOnly, EditFixedSize, Category = "WFC3D|Data",
		DisplayName = "Faces (Up, Back, Right, Left, Front, Down)")
	TArray<FString> Faces = {"", "", "", "", "", ""};

	FTileVariantInfo* TileVariations;

	bool operator==(const FBaseTileInfo& Other) const
	{
		return TileVariations == Other.TileVariations && Faces == Other.Faces;
	}

	bool operator!=(const FBaseTileInfo& Other) const
	{
		return !(*this == Other);
	}
};

/**
 * 데이터 에셋에 보여질 데이터 구조체
 * 베이스 타일 데이터 에셋
 * Visualization에 사용됨
 * BaseTileInfo는 중복되지 않고 전부 고유함
 * 타일 바리에이션 정보는 타일 바리에이션 정보 테이블에서 가져옴
 * Face는 순서대로 작성해야함.
 * 이게 필요가 없다.
 * 알고리즘으로 전부 생성한 다음 해당 타일의 id를 가져와서
 * VarientInfo에서 가져와야함.
 */
USTRUCT()
struct PROCEDURALWORLD_API FTileIDToTileVariantInfo
{
	GENERATED_BODY()
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "WFC3D|Data",
		DisplayName = "TileID -> TileVariantInfo")
	TMap<int32, FTileVariantInfo*> TileIDToTileVariantInfo;	
};

/**
 * 기본 타일 정보 데이터 테이블
 * 타일 정보를 작성하기 쉽게 CSV 파일로 읽어오기 위한 테이블
 */
USTRUCT()
struct PROCEDURALWORLD_API FBaseTileInfoDataTable : public FTableRowBase
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

/**
 * FacePair 저장용 자료구조
 * 알고리즘 계산에만 사용함
 */
USTRUCT(BlueprintType)
struct PROCEDURALWORLD_API FFacePair
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

/**
 * BitArray 저장용 자료구조
 * 알고리즘 계산에만 사용함
 */
USTRUCT(BlueprintType)
struct PROCEDURALWORLD_API FTileBitString
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
