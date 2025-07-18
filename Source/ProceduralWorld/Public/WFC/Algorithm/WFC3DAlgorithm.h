// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "WFC/Algorithm/WFC3DAlgorithmTypes.h"
#include "WFC/Algorithm/WFC3DCollapse.h"
#include "WFC/Algorithm/WFC3DPropagation.h"
#include "UObject/Object.h"
#include "WFC3DAlgorithm.generated.h"

/**
 * WFC3D 알고리즘의 핵심 실행 클래스
 * Wave Function Collapse 알고리즘의 메인 실행 로직을 담당합니다.
 */
UCLASS(BlueprintType, Blueprintable)
class PROCEDURALWORLD_API UWFC3DAlgorithm : public UObject
{
	GENERATED_BODY()

public:
	UWFC3DAlgorithm()
		: CollapseStrategy(FCollapseStrategy())
		  , PropagationStrategy(FPropagationStrategy())
		  , RandomStream(FRandomStream())
	{
	}

	UWFC3DAlgorithm(const FCollapseStrategy& InCollapseStrategy, const FPropagationStrategy& InPropagationStrategy)
		: CollapseStrategy(InCollapseStrategy)
		  , PropagationStrategy(InPropagationStrategy)
		  , RandomStream(FRandomStream())
	{
	}

	UWFC3DAlgorithm(const FCollapseStrategy& InCollapseStrategy, const FPropagationStrategy& InPropagationStrategy, const FRandomStream& InRandomStream)
	: CollapseStrategy(InCollapseStrategy)
	  , PropagationStrategy(InPropagationStrategy)
	  , RandomStream(InRandomStream)
	{
	}

	UFUNCTION(BlueprintCallable, Category = "WFCAlgorithm")
	FWFC3DResult Execute(const FWFC3DAlgorithmContext& Context);

	FCollapseStrategy CollapseStrategy;
	FPropagationStrategy PropagationStrategy;
	FRandomStream RandomStream;
};
