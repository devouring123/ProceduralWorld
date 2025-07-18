// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "WFC3DAlgorithmTypes.h"
#include "WFC3DAlgorithmMacros.h"
#include "WFC3DPropagation.generated.h"

struct FWFC3DCell;
struct FRandomStream;
struct FTileInfo;

class UWFC3DGrid;
class UWFC3DModelDataAsset;

/**
 * Propagation 전략 구조체 - 각 단계별 함수 포인터를 조합하여 전략 정의
 */
USTRUCT(BlueprintType)
struct PROCEDURALWORLD_API FPropagationStrategy
{
	GENERATED_BODY()

public:
	FPropagationStrategy()
		: RangeLimitStrategy(ERangeLimitStrategy::Disable)
	{
	}

	FPropagationStrategy(ERangeLimitStrategy InStrategy)
		: RangeLimitStrategy(InStrategy)
	{
	}

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WFC3D")
	ERangeLimitStrategy RangeLimitStrategy;
};

/**
 * WFC3D 알고리즘의 Propagate 함수 모음
 */
namespace WFC3DPropagateFunctions
{
	/**
	 * 전파 함수
	 * @param Context - WFC3D 전파 컨텍스트
	 * @param PropagationStrategy - 전파 전략
	 * @return FPropagationResult - 전파 결과
	 */
	FPropagationResult ExecutePropagation(const FWFC3DPropagationContext& Context, const FPropagationStrategy& PropagationStrategy);

	/**
	 * 단일 Cell 전파 함수
	 * @param PropagatedCell - 전파할 Cell
	 * @param Grid - WFC3D 그리드
	 * @param PropagationQueue - 전파 대기 큐
	 * @param ModelData - WFC3D 모델 데이터
	 */
	bool PropagateCell(FWFC3DCell* PropagatedCell, UWFC3DGrid* Grid, TQueue<FIntVector>& PropagationQueue, const UWFC3DModelDataAsset* ModelData);

	/**
	 * 전파 범위 제한 함수 모음
	 */
	namespace RangeLimit
	{
		/**
		 * 구형 범위 제한이 있는 경우
		 */
		DECLARE_PROPAGATOR_RANGE_LIMIT_STRATEGY(SphereRangeLimited);

		/**
		 * 큐브 제한이 있는 경우
		 */
		DECLARE_PROPAGATOR_RANGE_LIMIT_STRATEGY(CubeRangeLimited);
	}
};
