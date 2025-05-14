// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "WFC3DAlgorithmTypes.generated.h"

struct FCollapseStrategy;
struct FWFC3DCell;
struct FRandomStream;
struct FTileInfo;

class UWFC3DGrid;
class UWFC3DModelDataAsset;

/**
 * WFC3D 알고리즘 컨텍스트 - 모든 전략 함수에 공통적으로 사용되는 매개변수
 */
struct FWFC3DAlgorithmContext
{
	FWFC3DAlgorithmContext() = delete;

	FWFC3DAlgorithmContext(
		UWFC3DGrid* InGrid,
		const UWFC3DModelDataAsset* InModelData,
		const FRandomStream* InRandomStream)
		: Grid(InGrid),
		  ModelData(InModelData),
		  RandomStream(InRandomStream)
	{
	}

	/** Grid는 항상 수정 가능 해야 함 */
	mutable UWFC3DGrid* Grid;

	const UWFC3DModelDataAsset* ModelData;

	const FRandomStream* RandomStream;
};

struct FWFC3DPropagationContext : FWFC3DAlgorithmContext
{
	FWFC3DPropagationContext() = delete;

	FWFC3DPropagationContext(
		UWFC3DGrid* InGrid,
		const UWFC3DModelDataAsset* InModelData,
		const FRandomStream* InRandomStream,
		const FIntVector& InCollapseLocation,
		const int32 InRangeLimit)
		: FWFC3DAlgorithmContext(InGrid, InModelData, InRandomStream),
		  CollapseLocation(InCollapseLocation),
		  RangeLimit(InRangeLimit)
	{
	}

	const FIntVector CollapseLocation;

	const int32 RangeLimit;
};

/**
 * Collapse 결과 구조체
 */
USTRUCT(BlueprintType)
struct PROCEDURALWORLD_API FCollapseResult
{
	GENERATED_BODY()

public:
	FCollapseResult() = default;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "WFC3D")
	bool bSuccess = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "WFC3D")
	int32 CollapsedIndex = -1;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "WFC3D")
	FIntVector CollapsedLocation = FIntVector::ZeroValue;
};

/**
 * Propagation 결과 구조체
 */
USTRUCT(BlueprintType)
struct PROCEDURALWORLD_API FPropagationResult
{
	GENERATED_BODY()

public:
	FPropagationResult() = default;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "WFC3D")
	bool bSuccess = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "WFC3D")
	int32 AffectedCellCount = -1;
};

/**
 * 언리얼 엔진 델리게이트 시스템을 활용한 정적 함수 포인터 타입 정의
 */
template <typename T>
using TStaticFuncPtr = typename TBaseStaticDelegateInstance<T, FDefaultDelegateUserPolicy>::FFuncPtr;

/**
 * Cell 선택 함수 포인터 타입
 * @param FWFC3DAlgorithmContex& - WFC3D Algorithm Context
 * @return int32 - 선택된 셀 인덱스
 */

using SelectCellFunc = TStaticFuncPtr<int32(const FWFC3DAlgorithmContext&)>;

/**
 * TileInfo 선택 함수 포인터 타입
 * @param FWFC3DAlgorithmContex& - WFC3D Algorithm Context
 * @param int32 - 선택된 셀 인덱스
 * @return const FTileInfo* - 선택된 TileInfo
 */
using SelectTileInfoFunc = TStaticFuncPtr<const FTileInfo*(const FWFC3DAlgorithmContext&, const int32)>;

/**
 * 단일 Cell Collapse 함수 포인터 타입
 * @param FWFC3DCell* - 붕괴할 Cell
 * @param int32 - 붕괴할 Cell의 인덱스
 * @param FTileInfo* - 붕괴할 Cell에 들어갈 TileInfo
 * @return bool - 붕괴 성공 여부
 */
using CollapseSingleCellFunc = TStaticFuncPtr<bool(FWFC3DCell*, const int32, const FTileInfo*)>;

/**
 * Collapse 알고리즘 함수 포인터 타입
 * const FWFC3DAlgorithmContext&, FCollapseStrategy 매개변수를 받고
 * FCollapseResult를 반환하는 정적 함수 포인터
 */
using CollapseFunc = TStaticFuncPtr<FCollapseResult(const FWFC3DAlgorithmContext&, const FCollapseStrategy&)>;


/**
 * Propagation 알고리즘 함수 포인터 타입
 * const FWFC3DPropagationContext&, FPropagationStrategy, int32 RangeLimit 매개변수를 받고
 * FPropagationResult를 반환하는 정적 함수 포인터
 */
using PropagateFunc = TStaticFuncPtr<FPropagationResult(const FWFC3DPropagationContext&, const FPropagationStrategy&, const int32 RangeLimit)>;


/**
 * Collapse 셀 선택 전략 결과 열거형
 */
UENUM()
enum class ECollapseCellSelectStrategy : uint8
{
	/** 엔트로피 기반 셀 선택 */
	ByEntropy UMETA(DisplayName = "By Entropy"),

	/** 랜덤 셀 선택 */
	Random UMETA(DisplayName = "Random"),

	/** 사용자 정의 셀 선택 */
	Custom UMETA(DisplayName = "Custom")
};

/**
 * Collapse 타일 선택 전략 열거형
 */
UENUM()
enum class ECollapseTileInfoSelectStrategy : uint8
{
	/** 가중치 기반 타일 선택 */
	ByWeight UMETA(DisplayName = "By Weight"),

	/** 랜덤 타일 선택 */
	Random UMETA(DisplayName = "Random"),

	/** 사용자 정의 타일 선택 */
	Custom UMETA(DisplayName = "Custom")
};

/**
 * Collapse 셀 붕괴 전략 열거형
 */
UENUM()
enum class ECollapseSingleCellStrategy : uint8
{
	/** 기본 셀 붕괴 */
	Default UMETA(DisplayName = "Default"),

	/** 사용자 정의 셀 붕괴 */
	Custom UMETA(DisplayName = "Custom")
};


/** 
 * Propagation 전략 열거형 
 */
UENUM(BlueprintType)
enum class EPropagationStrategy : uint8
{
	/** 기본 Propagation */
	Standard UMETA(DisplayName = "Standard"),

	/** 범위 제한 Propagation */
	RangeLimited UMETA(DisplayName = "Range Limited"),

	/** 사용자 정의 Propagation */
	Custom UMETA(DisplayName = "Custom")
};

class FWFC3DHelperFunctions
{
public:
	/**
	 * 비트셋에서 모든 인덱스를 가져오는 함수
	 * @param Bitset - TBitArray<> 비트셋
	 * @return TArray<int32> - 비트셋에서 true인 모든 인덱스의 배열
	 */
	static TArray<int32> GetAllIndexFromBitset(const TBitArray<>& Bitset);

	/**
	 * 랜덤한 정수를 반환하는 함수
	 * @param Weights - 가중치 배열
	 * @param RandomStream - 랜덤 스트림
	 * @return int32 - 가중치에 따라 선택된 랜덤한 인덱스
	 */
	static int32 GetWeightedRandomIndex(const TArray<float>& Weights, const FRandomStream* RandomStream);

private:
	/** 유틸리티 클래스 생성자 및 소멸자 제거 */
	FWFC3DHelperFunctions() = delete;
	FWFC3DHelperFunctions(const FWFC3DHelperFunctions&) = delete;
	FWFC3DHelperFunctions& operator=(const FWFC3DHelperFunctions&) = delete;
	FWFC3DHelperFunctions(FWFC3DHelperFunctions&&) = delete;
	FWFC3DHelperFunctions& operator=(FWFC3DHelperFunctions&&) = delete;
	~FWFC3DHelperFunctions() = delete;
};
