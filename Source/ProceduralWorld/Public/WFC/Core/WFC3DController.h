// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "WFC/Data/WFC3DTypes.h"
#include "WFC/Data/WFC3DGrid.h"
#include "WFC/Data/WFC3DModelDataAsset.h"
#include "Engine/TimerHandle.h"
#include <atomic>

#include "WFC3DController.generated.h"

// Forward declarations
class UWFC3DAlgorithm;
class UWFC3DVisualizer;
class USceneComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnWFC3DExecutionCompleted, const FWFC3DAlgorithmResult&, Result);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnWFC3DExecutionCancelled);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnWFC3DExecutionProgress, int32, CurrentStep, int32, TotalSteps);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnWFC3DPhaseChanged, const FString&, PhaseName, float, PhaseProgress);

/**
 * WFC3D 실행 컨텍스트 구조체
 */
USTRUCT(BlueprintType)
struct FWFC3DExecutionContext
{
	GENERATED_BODY()

	FWFC3DExecutionContext()
		: GridDimension(FIntVector(5,5,5)),
		  ModelData(nullptr)
	{
	}

	/** 그리드 크기 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WFC3D")
	FIntVector GridDimension;

	/** 모델 데이터 에셋 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WFC3D")
	UWFC3DModelDataAsset* ModelData;

	/** 랜덤 시드 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WFC3D")
	int32 RandomSeed = 0;

	/** 시각화 활성화 여부 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WFC3D")
	bool bEnableVisualization = true;

	/** 실시간 생성 모드 (생성 과정을 단계별로 시각화) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WFC3D")
	bool bRealTimeGeneration = true;

	/** 알고리즘 실패 시 최대 재시도 횟수 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WFC3D", meta = (ClampMin = "1", ClampMax = "100000"))
	int32 MaxRetryCount = 1000;
};

/**
 * WFC3D 핵심 컨트롤러 클래스
 * Algorithm과 Visualizer를 통합하여 WFC 생성 과정을 관리합니다.
 */
UCLASS(BlueprintType, Blueprintable)
class PROCEDURALWORLD_API UWFC3DController : public UObject
{
	GENERATED_BODY()

public:
	UWFC3DController();

	/** 소멸자에서 리소스 정리 */
	virtual void BeginDestroy() override;

	/** WFC3D를 동기적으로 실행합니다 */
	UFUNCTION(BlueprintCallable, Category = "WFC3D")
	FWFC3DExecutionResult Execute(const FWFC3DExecutionContext& Context);

	/** WFC3D를 비동기적으로 실행합니다 */
	UFUNCTION(BlueprintCallable, Category = "WFC3D")
	void ExecuteAsync(const FWFC3DExecutionContext& Context);

	/** 실행 중인 WFC3D를 취소합니다 */
	UFUNCTION(BlueprintCallable, Category = "WFC3D")
	void CancelExecution();

	/** 실행 상태를 초기화합니다 */
	UFUNCTION(BlueprintCallable, Category = "WFC3D")
	void ResetExecutionState();

	/** 현재 실행 중인지 확인합니다 */
	UFUNCTION(BlueprintPure, Category = "WFC3D")
	bool IsRunning() const { return bIsRunning; }

	/** 현재 취소되었는지 확인합니다 */
	UFUNCTION(BlueprintPure, Category = "WFC3D")
	bool IsCancelled() const { return bIsCancelled; }

	/** 현재 완료되었는지 확인합니다 */
	UFUNCTION(BlueprintPure, Category = "WFC3D")
	bool IsComplete() const { return bIsComplete; }

	/** 현재 진행률을 반환합니다 (0.0 ~ 1.0) */
	UFUNCTION(BlueprintPure, Category = "WFC3D")
	float GetProgress() const;

	/** 현재 실행 단계 이름을 반환합니다 */
	UFUNCTION(BlueprintPure, Category = "WFC3D")
	FString GetCurrentPhaseName() const { return CurrentPhaseName; }

	/** 생성된 그리드를 반환합니다 */
	UFUNCTION(BlueprintPure, Category = "WFC3D")
	UWFC3DGrid* GetGeneratedGrid() const { return GeneratedGrid; }

	/** 시각화 컴포넌트를 반환합니다 */
	UFUNCTION(BlueprintPure, Category = "WFC3D")
	USceneComponent* GetVisualizationComponent() const;

	/** 실행 완료 시 호출되는 델리게이트 */
	UPROPERTY(BlueprintAssignable, Category = "WFC3D")
	FOnWFC3DExecutionCompleted OnExecutionCompleted;

	/** 실행 취소 시 호출되는 델리게이트 */
	UPROPERTY(BlueprintAssignable, Category = "WFC3D")
	FOnWFC3DExecutionCancelled OnExecutionCancelled;

	/** 실행 진행률 업데이트 시 호출되는 델리게이트 */
	UPROPERTY(BlueprintAssignable, Category = "WFC3D")
	FOnWFC3DExecutionProgress OnExecutionProgress;

	/** 실행 단계 변경 시 호출되는 델리게이트 */
	UPROPERTY(BlueprintAssignable, Category = "WFC3D")
	FOnWFC3DPhaseChanged OnPhaseChanged;

protected:
	/** 실행 상태 플래그들 */
	UPROPERTY(BlueprintReadOnly, Category = "WFC3D")
	bool bIsRunning;

	UPROPERTY(BlueprintReadOnly, Category = "WFC3D")
	bool bIsCancelled;

	UPROPERTY(BlueprintReadOnly, Category = "WFC3D")
	bool bIsComplete;

	/** 현재 실행 단계 이름 */
	UPROPERTY(BlueprintReadOnly, Category = "WFC3D")
	FString CurrentPhaseName;

	/** 생성된 그리드 */
	UPROPERTY(BlueprintReadOnly, Category = "WFC3D")
	UWFC3DGrid* GeneratedGrid;

	/** WFC3D 알고리즘 인스턴스 */
	UPROPERTY()
	UWFC3DAlgorithm* Algorithm;

	/** WFC3D 시각화 인스턴스 */
	UPROPERTY()
	UWFC3DVisualizer* Visualizer;

	/** 스레드 안전한 실행 상태 */
	std::atomic<bool> bIsRunningAtomic;
	std::atomic<bool> bIsCancelledAtomic;
	std::atomic<bool> bIsCompleteAtomic;
	std::atomic<int32> CurrentStepAtomic;
	std::atomic<int32> TotalStepsAtomic;

	/** 크리티컬 섹션 (스레드 동기화용) */
	mutable FCriticalSection CriticalSection;

	/** 비동기 작업 완료 체크용 타이머 핸들 */
	FTimerHandle AsyncCheckTimerHandle;

private:
	/** 내부 실행 함수 */
	FWFC3DExecutionResult ExecuteInternal(const FWFC3DExecutionContext& Context);

	/** 알고리즘 실행 단계 */
	FWFC3DAlgorithmResult ExecuteAlgorithm(const FWFC3DExecutionContext& Context);

	/** 시각화 실행 단계 */
	FWFC3DVisualizeResult ExecuteVisualization(const FWFC3DExecutionContext& Context);

	/** 비동기 작업 완료 체크 */
	void CheckAsyncCompletion(const FWFC3DExecutionContext& Context);

	/** 단계 변경 처리 */
	void SetCurrentPhase(const FString& PhaseName, float PhaseProgress = 0.0f);

	/** 진행률 업데이트 처리 */
	void UpdateProgress(int32 CurrentStep, int32 TotalSteps);

	/** 알고리즘 델리게이트 바인딩 */
	void BindAlgorithmDelegates();

	/** 시각화 델리게이트 바인딩 */
	void BindVisualizationDelegates();

	/** 델리게이트 언바인딩 */
	void UnbindDelegates();

	/** 알고리즘 진행률 콜백 */
	UFUNCTION()
	void OnAlgorithmProgress(int32 CurrentStep, int32 TotalSteps);

	/** 알고리즘 완료 콜백 */
	UFUNCTION()
	void OnAlgorithmCompleted(const FWFC3DAlgorithmResult& Result);

	/** 알고리즘 취소 콜백 */
	UFUNCTION()
	void OnAlgorithmCancelled();

	/** 시각화 진행률 콜백 */
	UFUNCTION()
	void OnVisualizationProgress(int32 CurrentStep, int32 TotalSteps);

	/** 시각화 완료 콜백 */
	UFUNCTION()
	void OnVisualizationCompleted(const FWFC3DVisualizeResult& Result);

	/** 시각화 취소 콜백 */
	UFUNCTION()
	void OnVisualizationCancelled();
};