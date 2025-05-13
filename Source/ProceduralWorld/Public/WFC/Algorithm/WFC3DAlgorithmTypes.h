// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "WFC3DAlgorithmTypes.generated.h"

struct FWFC3DCell;
struct FRandomStream;
struct FTileInfo;

class UWFC3DGrid;
class UWFC3DModelDataAsset;

/**
 * Collapse 결과 구조체
 */
USTRUCT(BlueprintType)
struct PROCEDURALWORLD_API FCollapseResult
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "WFC3D")
	bool bSuccess = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "WFC3D")
	int32 SelectedIndex;

	FCollapseResult() : bSuccess(false), SelectedIndex(-1)
	{
	}
};

/**
 * Propagation 결과 구조체
 */
USTRUCT(BlueprintType)
struct PROCEDURALWORLD_API FPropagationResult
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "WFC3D")
	bool bSuccess = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "WFC3D")
	int32 AffectedCellCount;

	FPropagationResult() : bSuccess(false), AffectedCellCount(0)
	{
	}
};

/**
 * 언리얼 엔진 델리게이트 시스템을 활용한 정적 함수 포인터 타입 정의
 */
template <typename T>
using TStaticFuncPtr = typename TBaseStaticDelegateInstance<T, FDefaultDelegateUserPolicy>::FFuncPtr;

/** TODO: 설명 추가 */
/**
 * Collapse 전략 함수 포인터 타입
 * UWFC3DGrid*, UWFC3DModelData*, FRandomStream&, int32 SelectedCellIndex 매개변수를 받고
 * const FTileInfo*를 반환하는 정적 함수 포인터
 */

using SelectCellFunc = TStaticFuncPtr<int32(UWFC3DGrid*, const FRandomStream&)>;

/**
 * TileInfo 선택 함수 포인터 타입
 * UWFC3DGrid*, UWFC3DModelData*, FRandomStream&, int32 SelectedCellIndex 매개변수를 받고
 * const FTileInfo*를 반환하는 정적 함수 포인터
 */
using SelectTileInfoFunc = TStaticFuncPtr<const FTileInfo*(UWFC3DGrid*, const UWFC3DModelDataAsset*, const FRandomStream&, int32)>;

/**
 * 단일 셀 붕괴 함수 포인터 타입
 * FWFC3DCell*, int32 SelectedCellIndex, FTileInfo* 매개변수를 받고
 * bool을 반환하는 정적 함수 포인터
 */
using CollapseSingleCellFunc = TStaticFuncPtr<bool(FWFC3DCell*, int32, const FTileInfo*)>;

/**
 * Collapse 알고리즘 함수 포인터 타입
 * UWFC3DGrid*, UWFC3DModelData*, FRandomStream& 매개변수를 받고
 * FCollapseResult를 반환하는 정적 함수 포인터
 */
using CollapseFunc = TStaticFuncPtr<FCollapseResult(UWFC3DGrid*, const UWFC3DModelDataAsset*, const FRandomStream&)>;

/**
 * Propagation 알고리즘 함수 포인터 타입
 * UWFC3DGrid*, UWFC3DModelData*, FRandomStream&, int32 RangeLimit 매개변수를 받고
 * FPropagationResult를 반환하는 정적 함수 포인터
 */
using PropagateFunc = TStaticFuncPtr<FPropagationResult(UWFC3DGrid*, const UWFC3DModelDataAsset*, const FRandomStream&, const int32 RangeLimit)>;

/** 
 * Collapse 전략 열거형 
 */
UENUM(BlueprintType)
enum class ECollapseStrategy : uint8
{
	/** 엔트로피와 가중치 기반 Collapse */
	Standard UMETA(DisplayName = "Standard"),
	
	/** 엔트로피 무시, 가중치 기반 Collapse */
	OnlyWeighted UMETA(DisplayName = "Only Weighted"),

	/** 엔트로피와 가중치 무시 전체 랜덤 Collapse */
	OnlyRandom UMETA(DisplayName = "Random"),
	
	/** 사용자 정의 Collapse */
	Custom UMETA(DisplayName = "Custom")
};

/**
 * Collapse 전략 결과 구조체
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

UENUM()
enum class ECollapseTileSelectStrategy : uint8
{
	/** 가중치 기반 타일 선택 */
	ByWeight UMETA(DisplayName = "By Weight"),

	/** 랜덤 타일 선택 */
	Random UMETA(DisplayName = "Random"),
	
	/** 사용자 정의 타일 선택 */
	Custom UMETA(DisplayName = "Custom")
};


UENUM()
enum class ECollapseCellCollapseStrategy : uint8
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
	static int32 GetWeightedRandomIndex(const TArray<float>& Weights, const FRandomStream& RandomStream);

private:
	/** 유틸리티 클래스 생성자 및 소멸자 삭제 */
	FWFC3DHelperFunctions() = delete;
	FWFC3DHelperFunctions(const FWFC3DHelperFunctions&) = delete;
	FWFC3DHelperFunctions& operator=(const FWFC3DHelperFunctions&) = delete;
	FWFC3DHelperFunctions(FWFC3DHelperFunctions&&) = delete;
	FWFC3DHelperFunctions& operator=(FWFC3DHelperFunctions&&) = delete;
	~FWFC3DHelperFunctions() = delete;

	
};

