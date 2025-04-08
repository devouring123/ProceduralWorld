// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Containers/BitArray.h"
#include "Engine/DataTable.h"
#include "WFC3DTypes.generated.h"

class UStaticMesh;
class UMaterialInterface;

/**
 * 3D 웨이브 함수 붕괴(Wave Function Collapse) 알고리즘에 사용되는 타입 정의
 * 이 파일은 WFC 3D 알고리즘 구현에 필요한 모든 타입을 포함합니다.
 */

/**
 * 3D 타일의 면 방향을 정의하는 열거형
 * 큐브 기반 WFC 구현에서 6개 면의 방향을 표현합니다.
 */
UENUM(BlueprintType)
enum class EFace : uint8
{
	Up = 0 UMETA(DisplayName = "Up"), // 상단 면
	Back = 1 UMETA(DisplayName = "Back"), // 뒷면
	Right = 2 UMETA(DisplayName = "Right"), // 오른쪽 면
	Left = 3 UMETA(DisplayName = "Left"), // 왼쪽 면
	Front = 4 UMETA(DisplayName = "Front"), // 앞면
	Down = 5 UMETA(DisplayName = "Down"), // 하단 면
	None = 6 UMETA(Hidden), // 미정의 면
};

/**
 * 타일의 기본 정보를 저장하는 구조체
 * WFC 알고리즘 계산에 사용됩니다.
 */
USTRUCT(BlueprintType)
struct PROCEDURALWORLD_API FTileInfo
{
	GENERATED_BODY()

public:
	
	FTileInfo() = default;

	/**
	 * @param InTileID - 타일의 고유 식별자
	 * @param InFaces - 각 면 정보
	 */
	FTileInfo(const int32& InTileID, const TArray<FString>& InFaces)
		: TileID(InTileID), Faces(InFaces)
	{
	}
	
	/** 타일의 고유 식별자 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "WFC3D|Data")
	int32 TileID = 0;

	/** 
	 * 각 면의 연결 정보
	 * 순서대로 Up, Back, Right, Left, Front, Down 면의 타입을 나타냅니다.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, EditFixedSize, Category = "WFC3D|Data",
		DisplayName = "Faces (Up, Back, Right, Left, Front, Down)")
	TArray<FString> Faces = {"", "", "", "", "", ""};

	/** 이 타일이 선택될 가중치 값 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "WFC3D|Data")
	float Weight = 1.0f;

	bool operator==(const FTileInfo& Other) const
	{
		return TileID == Other.TileID
			&& Faces == Other.Faces;
	}

	bool operator!=(const FTileInfo& Other) const
	{
		return !(*this == Other);
	}
};

/**
 * 타일 면의 방향과 타입 정보를 저장하는 구조체
 * WFC 알고리즘의 계산 및 제약 조건 설정에 사용됩니다.
 */
USTRUCT(BlueprintType)
struct PROCEDURALWORLD_API FFaceInfo
{
	GENERATED_BODY()

public:
	
	FFaceInfo() = default;

	/**
	 * @param InDirection - 면의 방향
	 * @param InName - 면의 타입 이름
	 */
	FFaceInfo(const EFace& InDirection, const FString& InName)
		: Direction(InDirection), Name(InName)
	{
	}

	/** 면의 방향 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WFC3D|Data")
	EFace Direction = EFace::None;

	/** 면의 타입 이름 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WFC3D|Data")
	FString Name;

	bool operator==(const FFaceInfo& Other) const
	{
		return Direction == Other.Direction && Name == Other.Name;
	}

	bool operator!=(const FFaceInfo& Other) const
	{
		return !(*this == Other);
	}

	friend uint32 GetTypeHash(const FFaceInfo& FaceInfo)
	{
		return HashCombine(GetTypeHash(FaceInfo.Direction), GetTypeHash(FaceInfo.Name));
	}
};

/**
 * 타일의 각 면에 대한 인덱스 정보를 저장하는 구조체
 * TileToFaceMap의 Value로 사용되며, WFC 알고리즘 계산에 사용됩니다.
 */
USTRUCT(BlueprintType)
struct PROCEDURALWORLD_API FTileFaceIndices
{
	GENERATED_BODY()

public:
	
	FTileFaceIndices() = default;

	/**
	 * @param InFaceIndices - 이동될 면 인덱스 배열
	 */
	FTileFaceIndices(TArray<int32>&& InFaceIndices) : FaceIndices(InFaceIndices)
	{
	}

	/** 
	 * 각 면의 인덱스 배열 
	 * 고정 크기 6으로, 순서대로 Up, Back, Right, Left, Front, Down 면의 인덱스입니다.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, EditFixedSize, Category = "WFC3D|Data",
		DisplayName = "FaceIndices (Up, Back, Right, Left, Front, Down)")
	TArray<int32> FaceIndices = {0, 0, 0, 0, 0, 0};
};

/**
 * 비트 배열을 문자열 형태로 저장하는 구조체
 * WFC 알고리즘의 상태 저장 및 직렬화에 사용됩니다.
 */
USTRUCT(BlueprintType)
struct PROCEDURALWORLD_API FTileBitString
{
	GENERATED_BODY()

public:
	
	FTileBitString() = default;

	/**
	 * @param InBitArray - 변환할 비트 배열
	 */
	FTileBitString(const TBitArray<>& InBitArray)
	{
		BitString = ToString(InBitArray);
	}

	/**
	 * 저장된 비트 문자열을 비트 배열로 변환하여 반환
	 * @return 변환된 비트 배열
	 */
	TBitArray<> GetBitArray() const
	{
		return ToBitArray(BitString);
	}

	/**
	 * 비트 개수 반환
	 * @return 비트 문자열의 길이
	 */
	int32 Num() const
	{
		return BitString.Len();
	}

	/**
	 * 비트 배열을 문자열로 변환
	 * @param BitArray - 변환할 비트 배열
	 * @return 비트를 나타내는 0과 1로 구성된 문자열
	 */
	static FString ToString(const TBitArray<>& BitArray)
	{
		FString BitString;
		for (uint8 i = 0; i < BitArray.Num(); ++i)
		{
			BitString += BitArray[i] ? TEXT("1") : TEXT("0");
		}
		return BitString;
	}

	/**
	 * 문자열을 비트 배열로 변환
	 * @param BitString - 0과 1로 구성된 비트 문자열
	 * @return 변환된 비트 배열
	 */
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

private:
	/** 비트 배열을 표현하는 0과 1로 구성된 문자열 */
	UPROPERTY(VisibleAnywhere, Category = "WFC3D|Data")
	FString BitString;
};

/**
 * 타일의 시각적 정보를 저장하는 구조체
 * 생성된 WFC 결과를 시각화하는 데 사용됩니다.
 */
USTRUCT(BlueprintType)
struct PROCEDURALWORLD_API FTileVisualInfo
{
	GENERATED_BODY()

public:
	
	FTileVisualInfo() = default;

	/** 타일의 스태틱 메시 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "WFC3D|Data")
	UStaticMesh* Mesh = nullptr;

	/** 타일에 적용할 재질 배열 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "WFC3D|Data")
	TArray<UMaterialInterface*> Materials;

	/** 변형 정보 참조 - 내부 데이터 연결용 */
	FBaseTileInfo* VariantInfo = nullptr;
};

/**
 * 단일 타일의 시각 정보를 저장하는 구조체
 * 시각화에 사용됩니다.
 */
USTRUCT(BlueprintType)
struct PROCEDURALWORLD_API FTile
{
	GENERATED_BODY()

public:
	
	FTile() = default;

	/** 타일의 스태틱 메시 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "WFC3D|Data")
	UStaticMesh* StaticMesh = nullptr;

	/** 타일에 적용할 재질 배열 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "WFC3D|Data")
	TArray<UMaterialInterface*> Materials{};

	/** 이 타일이 선택될 가중치 값 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "WFC3D|Data")
	float Weight = 1.0f;
};

/**
 * 바이옴별 타일 정보를 저장하는 구조체
 * 동일한 타일 정보를 가진 다양한 시각적 표현을 제공합니다.
 * 시각화에 사용됩니다.
 */
USTRUCT(BlueprintType)
struct PROCEDURALWORLD_API FTileByBiome
{
	GENERATED_BODY()

public:
	
	FTileByBiome() = default;

	/** 바이옴에 속한 타일 변형 배열 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "WFC3D|Data")
	TArray<FTile> Tiles;

	/** 모든 타일 변형의 총 가중치 합계 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "WFC3D|Data")
	float TotalWeight = 0.0f;

	/**
	 * 총 가중치를 계산하는 메서드
	 * 모든 타일 변형의 가중치를 합산합니다.
	 */
	void CalculateTotalWeight();
};

/**
 * 바이옴별 타일 변형 정보를 저장하는 구조체
 * 모든 바이옴에 대한 타일 변형을 관리합니다.
 * 시각화에 사용됩니다.
 */
USTRUCT(BlueprintType)
struct PROCEDURALWORLD_API FTileVariantInfo
{
	GENERATED_BODY()

public:
	
	FTileVariantInfo() = default;

	/** 바이옴 이름을 키로 사용하는 바이옴별 타일 정보 맵 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "WFC3D|Data")
	TMap<FString, FTileByBiome> Biomes;

	/**
	 * 모든 바이옴 타일의 총 가중치를 계산하는 메서드
	 */
	void CalculateTotalWeight();
};

/**
 * 타일 변형 레지스트리 구조체
 * 모든 타일 ID와 해당 변형 정보를 맵핑합니다.
 * 데이터 에셋에 저장되며 시각화에 사용됩니다.
 */
USTRUCT(BlueprintType)
struct PROCEDURALWORLD_API FTileVariantRegistry
{
	GENERATED_BODY()

public:
	
	FTileVariantRegistry() = default;

	/** 타일 ID를 키로 사용하는 타일 변형 정보 맵 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "WFC3D|Data",
		DisplayName = "TileID -> TileVariantInfo")
	TMap<int32, FTileVariantInfo> Variants;
};

/**
 * 타일 정보 테이블 구조체
 * CSV 파일에서 타일 정보를 읽어오기 위한 DataTable 구조체입니다.
 */
USTRUCT()
struct PROCEDURALWORLD_API FTileInfoTable : public FTableRowBase
{
	GENERATED_BODY()

public:
	
	FTileInfoTable() = default;

	/** 타일의 선택 가중치 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "WFC3D|Data")
	float TileWeight = 1.0f;

	/** 상단 면 타입 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "WFC3D|Data")
	FString Up = "";

	/** 뒷면 타입 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "WFC3D|Data")
	FString Back = "";

	/** 오른쪽 면 타입 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "WFC3D|Data")
	FString Right = "";

	/** 왼쪽 면 타입 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "WFC3D|Data")
	FString Left = "";

	/** 앞면 타입 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "WFC3D|Data")
	FString Front = "";

	/** 하단 면 타입 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "WFC3D|Data")
	FString Down = "";
};

/**
 * 타일 변형 정보 테이블 구조체
 * CSV 파일에서 타일 변형 정보를 읽어오기 위한 DataTable 구조체입니다.
 */
USTRUCT()
struct PROCEDURALWORLD_API FTileVariantInfoTable : public FTableRowBase
{
	GENERATED_BODY()

public:
	
	FTileVariantInfoTable() = default;

	/** 타일의 이름 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "WFC3D|Data")
	FString TileName = "";

	/** 바이옴의 이름 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "WFC3D|Data")
	FString BiomeName = "";

	/** 타일의 스태틱 메시 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "WFC3D|Data")
	UStaticMesh* TileMesh;

	/** 타일에 적용할 재질 배열 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "WFC3D|Data")
	TArray<UMaterialInterface*> Materials;

	/** 이 타일 변형이 선택될 가중치 값 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "WFC3D|Data")
	float Weight = 1.0f;
};
