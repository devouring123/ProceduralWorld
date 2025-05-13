// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "WFC3DAlgorithmTypes.h"
#include "UObject/Object.h"
#include "WFC3DPropagation.generated.h"

/**
 * WFC3D 알고리즘의 Propagate 함수 모음
 */
namespace PROCEDURALWORLD_API WFC3DPropagateFunctions
{
	/**
	 * Cell 전파 함수
	 * @param Location - 전파할 Cell의 위치
	 * @param Grid - WFC3D 그리드
	 * @param ModelData - WFC3D 모델 데이터
	 */
	bool PropagateCell(const FIntVector& Location, UWFC3DGrid* Grid, const UWFC3DModelDataAsset* ModelData);
	
	/**
	 * 기본 Propagation 함수
	 */
	FPropagationResult StandardPropagate(UWFC3DGrid* Grid, const UWFC3DModelDataAsset* ModelData, const FRandomStream& RandomStream, const int32 RangeLimit = 0);

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
