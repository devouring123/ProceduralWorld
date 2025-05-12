// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "WFC3DAlgorithmTypes.h"
#include "UObject/Object.h"
#include "WFC3DPropagation.generated.h"

/**
 * WFC3D 알고리즘의 Propagate 함수 모음
 */
namespace WFC3DPropagateFunctions
{
	/**
	 * 기본 Propagation 함수
	 */
	FPropagationResult StandardPropagate(UWFC3DGrid* Grid, UWFC3DModelDataAsset* ModelData, FRandomStream& RandomStream, const int32 RangeLimit = 0);

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
