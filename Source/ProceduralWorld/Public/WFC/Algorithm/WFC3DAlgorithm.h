// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "WFC3DCollapse.h"
#include "WFC3DPropagation.h"
#include "UObject/Object.h"
#include "WFC3DAlgorithm.generated.h"

struct FCollapseResult;
struct FPropagationResult;


USTRUCT(BlueprintType)
struct FWFC3DResult
{
	GENERATED_BODY()

	bool bSuccess = false;
	FCollapseResult CollapseResults;
	TArray<FPropagationResult> PropagationResults;
};


/**
 * WFC3D 알고리즘의 핵심 실행 클래스
 * Wave Function Collapse 알고리즘의 메인 실행 로직을 담당합니다.
 */
UCLASS(BlueprintType, Blueprintable)
class PROCEDURALWORLD_API UWFC3DAlgorithm : public UObject
{
	GENERATED_BODY()

public:
	
	UWFC3DAlgorithm();
	UWFC3DAlgorithm(const FCollapseStrategy& InCollapseStrategy, const FPropagationStrategy& InPropagationStrategy);

	UFUNCTION(BlueprintCallable, Category = "WFCAlgorithm")
	FWFC3DResult Execute(const FWFC3DAlgorithmContext& Context);
	
	FCollapseStrategy CollapseStrategy;
	FPropagationStrategy PropagationStrategy;

	FRandomStream RandomStream;



	
};
