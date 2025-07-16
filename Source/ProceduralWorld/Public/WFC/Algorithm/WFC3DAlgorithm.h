// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "WFC3DAlgorithmTypes.h"
#include "WFC3DCollapse.h"
#include "WFC3DPropagation.h"
#include "UObject/Object.h"
#include "WFC3DAlgorithm.generated.h"

class UWFC3DGrid;
class UWFC3DModelDataAsset;
class UWFC3DController;
struct FCollapseStrategy;
struct FPropagationStrategy;
struct FRandomStream;

DECLARE_DELEGATE(FOnAlgorithmCompleted);
DECLARE_DELEGATE_OneParam(FOnAlgorithmFailed, const FString&);
DECLARE_DELEGATE_OneParam(FOnAlgorithmProgress, float);

/**
 * WFC3D 알고리즘의 핵심 실행 클래스
 * Wave Function Collapse 알고리즘의 메인 실행 로직을 담당합니다.
 */
UCLASS(BlueprintType, Blueprintable)
class PROCEDURALWORLD_API UWFC3DAlgorithm : public UObject
{
	GENERATED_BODY()

public:
	/** 알고리즘 초기화 */
	UFUNCTION(BlueprintCallable, Category = "WFC3D|Algorithm")
	void Initialize(UWFC3DGrid* InGrid, const UWFC3DModelDataAsset* InModelData, int32 InRandomSeed = 0);

	/** 알고리즘 실행 */
	UFUNCTION(BlueprintCallable, Category = "WFC3D|Algorithm")
	bool Execute(int32 MaxIterations = -1, int32 StepsPerIteration = 1);

	/** 알고리즘 중지 */
	UFUNCTION(BlueprintCallable, Category = "WFC3D|Algorithm")
	void Stop();

	/** 알고리즘 리셋 */
	UFUNCTION(BlueprintCallable, Category = "WFC3D|Algorithm")
	void Reset();

	/** 알고리즘 완료 여부 확인 */
	UFUNCTION(BlueprintPure, Category = "WFC3D|Algorithm")
	bool IsCompleted() const { return bIsCompleted; }

	/** 알고리즘 실행 중 여부 확인 */
	UFUNCTION(BlueprintPure, Category = "WFC3D|Algorithm")
	bool IsRunning() const { return bIsRunning; }

	/** 현재 진행도 확인 (0.0 ~ 1.0) */
	UFUNCTION(BlueprintPure, Category = "WFC3D|Algorithm")
	float GetProgress() const { return Progress; }

	/** 현재 반복 횟수 확인 */
	UFUNCTION(BlueprintPure, Category = "WFC3D|Algorithm")
	int32 GetCurrentIteration() const { return CurrentIteration; }

	/** Collapse 전략 설정 */
	UFUNCTION(BlueprintCallable, Category = "WFC3D|Algorithm")
	void SetCollapseStrategy(const FCollapseStrategy& InStrategy);

	/** Propagation 전략 설정 */
	UFUNCTION(BlueprintCallable, Category = "WFC3D|Algorithm")
	void SetPropagationStrategy(const FPropagationStrategy& InStrategy);

	/** 전파 범위 제한 설정 */
	UFUNCTION(BlueprintCallable, Category = "WFC3D|Algorithm")
	void SetPropagationRangeLimit(int32 InRangeLimit) { PropagationRangeLimit = InRangeLimit; }

public:
	/** 알고리즘 완료 델리게이트 */
	FOnAlgorithmCompleted OnAlgorithmCompleted;

	/** 알고리즘 실패 델리게이트 */
	FOnAlgorithmFailed OnAlgorithmFailed;

	/** 알고리즘 진행도 업데이트 델리게이트 */
	FOnAlgorithmProgress OnAlgorithmProgress;

protected:
	/** 알고리즘 단일 스텝 실행 */
	virtual bool ExecuteStep();

	/** 가장 낮은 엔트로피를 가진 셀 찾기 */
	virtual int32 FindCellWithLowestEntropy() const;

	/** Collapse 실행 */
	virtual bool ExecuteCollapse();

	/** Propagation 실행 */
	virtual bool ExecutePropagation(const FIntVector& CollapseLocation);

	/** 알고리즘 실패 처리 */
	virtual void HandleFailure(const FString& FailureReason);

	/** 진행도 업데이트 */
	virtual void UpdateProgress();

	/** 알고리즘 초기화 검증 */
	bool ValidateInitialization() const;

protected:
	/** WFC3D 그리드 참조 */
	UPROPERTY(BlueprintReadOnly, Category = "WFC3D|Algorithm")
	UWFC3DGrid* Grid;

	/** WFC3D 모델 데이터 참조 */
	UPROPERTY(BlueprintReadOnly, Category = "WFC3D|Algorithm")
	const UWFC3DModelDataAsset* ModelData;

	/** Collapse 전략 */
	UPROPERTY(BlueprintReadOnly, Category = "WFC3D|Algorithm")
	FCollapseStrategy CollapseStrategy;

	/** Propagation 전략 */
	UPROPERTY(BlueprintReadOnly, Category = "WFC3D|Algorithm")
	FPropagationStrategy PropagationStrategy;

	/** 랜덤 스트림 */
	FRandomStream RandomStream;

	/** 알고리즘 완료 여부 */
	UPROPERTY(BlueprintReadOnly, Category = "WFC3D|Algorithm")
	bool bIsCompleted;

	/** 알고리즘 실행 중 여부 */
	UPROPERTY(BlueprintReadOnly, Category = "WFC3D|Algorithm")
	bool bIsRunning;

	/** 알고리즘 초기화 여부 */
	UPROPERTY(BlueprintReadOnly, Category = "WFC3D|Algorithm")
	bool bIsInitialized;

	/** 현재 진행도 (0.0 ~ 1.0) */
	UPROPERTY(BlueprintReadOnly, Category = "WFC3D|Algorithm")
	float Progress;

	/** 현재 반복 횟수 */
	UPROPERTY(BlueprintReadOnly, Category = "WFC3D|Algorithm")
	int32 CurrentIteration;

	/** 최대 반복 횟수 */
	UPROPERTY(BlueprintReadOnly, Category = "WFC3D|Algorithm")
	int32 MaxIterations;

	/** 전파 범위 제한 */
	UPROPERTY(BlueprintReadOnly, Category = "WFC3D|Algorithm")
	int32 PropagationRangeLimit;

	/** 마지막 Collapse 위치 */
	FIntVector LastCollapseLocation;

	/** 백트래킹을 위한 히스토리 (확장 기능용) */
	TArray<FCollapseCellResult> CollapseHistory;
};
