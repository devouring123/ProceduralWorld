// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "WFC3DAlgorithmTypes.generated.h"

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

/**
 * Collapse 알고리즘 함수 포인터 타입
 * UWFC3DGrid*, UWFC3DModelData*, FRandomStream& 매개변수를 받고
 * FCollapseResult를 반환하는 정적 함수 포인터
 */
using CollapseFunc = TStaticFuncPtr<FCollapseResult(UWFC3DGrid*, const UWFC3DModelDataAsset*, FRandomStream&)>;

/**
 * Propagation 알고리즘 함수 포인터 타입
 * UWFC3DGrid*, UWFC3DModelData*, FRandomStream&, int32 RangeLimit 매개변수를 받고
 * FPropagationResult를 반환하는 정적 함수 포인터
 */
using PropagateFunc = TStaticFuncPtr<FPropagationResult(UWFC3DGrid*, const UWFC3DModelDataAsset*, FRandomStream&, const int32 RangeLimit)>;

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

namespace WFC3DHelperFunctions
{
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

}

