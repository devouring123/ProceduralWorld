// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "WFC3DAlgorithmTypes.h"
#include "UObject/Object.h"
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
};


/**
 * WFC3D 알고리즘의 Propagate 함수 모음
 */
namespace
PROCEDURALWORLD_API WFC3DPropagateFunctions
{
	/**
	 * Cell 전파 함수
	 * @param Location - 전파할 Cell의 위치
	 * @param Grid - WFC3D 그리드
	 * @param ModelData - WFC3D 모델 데이터
	 */
	bool PropagateSingleCell(const FIntVector& Location, UWFC3DGrid* Grid, const UWFC3DModelDataAsset* ModelData);

	/**
	 * 전파할 위치가 유효한지 검사하는 함수
	 * @param Location - 위치
	 * @param Dimension - 그리드 크기
	 * @return 
	 */
	FORCEINLINE bool IsValidLocation(const FIntVector& Location, const FIntVector& Dimension)
	{
		return Location.X >= 0 && Location.Y >= 0 && Location.Z >= 0 && Location.X < Dimension.X && Location.Y < Dimension.Y && Location.Z < Dimension.Z;
	}

	/**
	 * 기본 Propagation 함수
	 */
	FPropagationResult StandardPropagate(const FWFC3DPropagationContext& Context, const FPropagationStrategy& PropagationStrategy, const int32 RangeLimit = 0);

	// /**
	//  * 범위 제한 Propagation 함수
	//  */
	// FPropagationResult RangeLimitedPropagate(UWFC3DGrid* Grid, UWFC3DModelDataAsset* ModelData, FRandomStream& RandomStream, const int32 RangeLimit = 5);

	// /**
	//  * 커스텀 Propagation 함수
	//  */
	// FPropagationResult CustomPropagate(UWFC3DGrid* Grid, UWFC3DModelDataAsset* ModelData, FRandomStream& RandomStream, const int32 RangeLimit = 0);

	/**
	 * 전략에 따른 Propagation 함수 포인터 반환
	 */
	PropagateFunc GetPropagateFunction(EPropagationStrategy Strategy);
};
