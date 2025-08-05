// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "WFC/Data/WFC3DTypes.h"
#include "WFC/Algorithm/WFC3DCollapse.h"
#include "WFC/Algorithm/WFC3DPropagation.h"
#include "UObject/Object.h"
#include "HAL/Runnable.h"
#include "HAL/RunnableThread.h"
#include "Async/AsyncWork.h"
#include "Engine/Engine.h"
#include "Engine/TimerHandle.h"
#include <atomic>
#include "WFC3DAlgorithm.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnWFC3DAlgorithmCompleted, const FWFC3DAlgorithmResult&, Result);

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnWFC3DAlgorithmCancelled);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnWFC3DAlgorithmProgress, int32, CurrentStep, int32, TotalSteps);

// 전방 선언
class UWFC3DAlgorithm;

/**
 * WFC3D 알고리즘 비동기 작업 클래스
 * AsyncTask 시스템을 사용하여 백그라운드에서 알고리즘을 실행합니다.
 */
class FWFC3DAlgorithmAsyncTask : public FNonAbandonableTask
{
public:
	FWFC3DAlgorithmAsyncTask(UWFC3DAlgorithm* InAlgorithm, const FWFC3DAlgorithmContext& InContext)
		: Algorithm(InAlgorithm),
		  Context(InContext),
		  Result()
	{
	}

	/** 비동기 작업에서 실행될 함수 */
	void DoWork();

	/** 태스크 통계 정보 */
	FORCEINLINE TStatId GetStatId() const
	{
		RETURN_QUICK_DECLARE_CYCLE_STAT(FWFC3DAsyncTask, STATGROUP_ThreadPoolAsyncTasks);
	}

	/** 결과 가져오기 */
	FWFC3DAlgorithmResult GetResult() const { return Result; }

private:
	UWFC3DAlgorithm* Algorithm;
	FWFC3DAlgorithmContext Context;
	FWFC3DAlgorithmResult Result;
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
	UWFC3DAlgorithm()
		: CollapseStrategy(FCollapseStrategy())
		  , PropagationStrategy(FPropagationStrategy())
		  , RandomStream(FRandomStream())
		  , bIsRunning(false)
		  , bIsCancelled(false)
		  , bIsComplete(false)
		  , CurrentStep(0)
		  , TotalSteps(0)
		  , AsyncTask(nullptr)
	{
	}

	UWFC3DAlgorithm(const FCollapseStrategy& InCollapseStrategy, const FPropagationStrategy& InPropagationStrategy)
		: CollapseStrategy(InCollapseStrategy)
		  , PropagationStrategy(InPropagationStrategy)
		  , RandomStream(FRandomStream())
		  , bIsRunning(false)
		  , bIsCancelled(false)
		  , bIsComplete(false)
		  , CurrentStep(0)
		  , TotalSteps(0)
		  , AsyncTask(nullptr)
	{
	}

	UWFC3DAlgorithm(const FCollapseStrategy& InCollapseStrategy, const FPropagationStrategy& InPropagationStrategy, const FRandomStream& InRandomStream)
		: CollapseStrategy(InCollapseStrategy)
		  , PropagationStrategy(InPropagationStrategy)
		  , RandomStream(InRandomStream)
		  , bIsRunning(false)
		  , bIsCancelled(false)
		  , bIsComplete(false)
		  , CurrentStep(0)
		  , TotalSteps(0)
		  , AsyncTask(nullptr)
	{
	}

	/** 소멸자에서 스레드 정리 */
	virtual void BeginDestroy() override;

	/** WFC3D 알고리즘을 동기적으로 실행합니다 (메인 스레드) */
	UFUNCTION(BlueprintCallable, Category = "WFCAlgorithm")
	FWFC3DAlgorithmResult Execute(const FWFC3DAlgorithmContext& Context);

	/** WFC3D 알고리즘을 비동기적으로 실행합니다 (백그라운드 스레드) */
	UFUNCTION(BlueprintCallable, Category = "WFCAlgorithm")
	void ExecuteAsync(const FWFC3DAlgorithmContext& Context);

	/** 실행 중인 알고리즘을 취소합니다 */
	UFUNCTION(BlueprintCallable, Category = "WFCAlgorithm")
	void CancelExecution();

	/** 알고리즘 실행 상태를 초기화합니다 */
	UFUNCTION(BlueprintCallable, Category = "WFCAlgorithm")
	void ResetExecutionState();

	/** 현재 알고리즘이 실행 중인지 확인합니다 */
	UFUNCTION(BlueprintPure, Category = "WFCAlgorithm")
	bool IsRunning() const { return bIsRunning; }

	/** 현재 알고리즘이 취소되었는지 확인합니다 */
	UFUNCTION(BlueprintPure, Category = "WFCAlgorithm")
	bool IsCancelled() const { return bIsCancelled; }

	/** 현재 알고리즘이 완료되었는지 확인합니다 */
	UFUNCTION(BlueprintPure, Category = "WFCAlgorithm")
	bool IsComplete() const { return bIsComplete; }

	/** 현재 진행률을 반환합니다 (0.0 ~ 1.0) */
	UFUNCTION(BlueprintPure, Category = "WFCAlgorithm")
	float GetProgress() const;

	/** 내부 실행 함수 (스레드 안전) */
	FWFC3DAlgorithmResult ExecuteInternal(const FWFC3DAlgorithmContext& Context);

	/** 비동기 작업 상태를 확인하고 결과를 처리합니다 */
	UFUNCTION(BlueprintCallable, Category = "WFCAlgorithm")
	void CheckAsyncTaskCompletion();

	FCollapseStrategy CollapseStrategy;
	FPropagationStrategy PropagationStrategy;
	FRandomStream RandomStream;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WFCAlgorithm")
	int32 Seed = 0;

	/** 알고리즘 완료 시 호출되는 델리게이트 */
	UPROPERTY(BlueprintAssignable, Category = "WFCAlgorithm")
	FOnWFC3DAlgorithmCompleted OnAlgorithmCompleted;

	/** 알고리즘 취소 시 호출되는 델리게이트 */
	UPROPERTY(BlueprintAssignable, Category = "WFCAlgorithm")
	FOnWFC3DAlgorithmCancelled OnAlgorithmCancelled;

	/** 알고리즘 진행률 업데이트 시 호출되는 델리게이트 */
	UPROPERTY(BlueprintAssignable, Category = "WFCAlgorithm")
	FOnWFC3DAlgorithmProgress OnAlgorithmProgress;

protected:
	/** 알고리즘이 현재 실행 중인지 나타내는 플래그 */
	UPROPERTY(BlueprintReadOnly, Category = "WFCAlgorithm")
	bool bIsRunning;

	/** 알고리즘 실행이 취소되었는지 나타내는 플래그 */
	UPROPERTY(BlueprintReadOnly, Category = "WFCAlgorithm")
	bool bIsCancelled;

	/** 알고리즘 실행이 완료되었는지 나타내는 플래그 */
	UPROPERTY(BlueprintReadOnly, Category = "WFCAlgorithm")
	bool bIsComplete;

	/** 현재 진행 단계 */
	UPROPERTY(BlueprintReadOnly, Category = "WFCAlgorithm")
	int32 CurrentStep;

	/** 전체 단계 수 */
	UPROPERTY(BlueprintReadOnly, Category = "WFCAlgorithm")
	int32 TotalSteps;

	/** 스레드 안전한 실행 상태 */
	std::atomic<bool> bIsRunningAtomic;
	std::atomic<bool> bIsCancelledAtomic;
	std::atomic<bool> bIsCompleteAtomic;
	std::atomic<int32> CurrentStepAtomic;
	std::atomic<int32> TotalStepsAtomic;

	/** 비동기 태스크 인스턴스 */
	TUniquePtr<FAsyncTask<FWFC3DAlgorithmAsyncTask>> AsyncTask;

	/** 크리티컬 섹션 (스레드 동기화용) */
	mutable FCriticalSection CriticalSection;

	/** 비동기 작업 완료 체크용 타이머 핸들 */
	FTimerHandle AsyncCheckTimerHandle;
};
