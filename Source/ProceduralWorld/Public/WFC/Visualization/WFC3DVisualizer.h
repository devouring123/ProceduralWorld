// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "WFC/Data/WFC3DTypes.h"
#include "UObject/Object.h"
#include "HAL/Runnable.h"
#include "HAL/RunnableThread.h"
#include "Async/AsyncWork.h"
#include "Engine/Engine.h"
#include "Engine/TimerHandle.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SceneComponent.h"
#include "Engine/StaticMesh.h"
#include "Materials/MaterialInterface.h"
#include "Engine/World.h"
#include <atomic>
#include "WFC3DVisualizer.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnWFC3DVisualizationCompleted, const FWFC3DVisualizeResult&, Result);

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnWFC3DVisualizationCancelled);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnWFC3DVisualizationProgress, int32, CurrentStep, int32, TotalSteps);



/**
 * 시각화 컨텍스트 구조체
 */
USTRUCT(BlueprintType)
struct FWFC3DVisualizeContext
{
	GENERATED_BODY()
	FWFC3DVisualizeContext()
		: Grid(nullptr),
		  ModelData(nullptr),
		  RandomStream(nullptr)		  
	{
	}

	FWFC3DVisualizeContext(
		UWFC3DGrid* InGrid,
		const UWFC3DModelDataAsset* InModelData,
		const FRandomStream* InRandomStream)
		: Grid(InGrid),
		  ModelData(InModelData),
		  RandomStream(InRandomStream)
	{
	}

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WFC3D")
	mutable UWFC3DGrid* Grid;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WFC3D")
	const UWFC3DModelDataAsset* ModelData;
	
	const FRandomStream* RandomStream;
};

/**
 * WFC3D 시각화 비동기 작업 클래스
 * AsyncTask 시스템을 사용하여 백그라운드에서 시각화를 실행합니다.
 */
class FWFC3DVisualizeAsyncTask : public FNonAbandonableTask
{
public:
	FWFC3DVisualizeAsyncTask(UWFC3DVisualizer* InVisualizer, const FWFC3DVisualizeContext& InContext)
		: Visualizer(InVisualizer),
		  Context(InContext),
		  Result()
	{
	}

	/** 비동기 작업에서 실행될 함수 */
	void DoWork();

	/** 태스크 통계 정보 */
	FORCEINLINE TStatId GetStatId() const
	{
		RETURN_QUICK_DECLARE_CYCLE_STAT(FWFC3DVisualizeAsyncTask, STATGROUP_ThreadPoolAsyncTasks);
	}

	/** 결과 가져오기 */
	FWFC3DVisualizeResult GetResult() const { return Result; }

private:
	UWFC3DVisualizer* Visualizer;
	FWFC3DVisualizeContext Context;
	FWFC3DVisualizeResult Result;
};


/**
 * WFC3D 시각화의 핵심 실행 클래스
 * Wave Function Collapse 결과를 시각적으로 표현하는 로직을 담당합니다.
 */
UCLASS(BlueprintType, Blueprintable)
class PROCEDURALWORLD_API UWFC3DVisualizer : public UObject
{
	GENERATED_BODY()

public:
	UWFC3DVisualizer()
		: bIsRunning(false)
		  , bIsCancelled(false)
		  , bIsComplete(false)
		  , CurrentStep(0)
		  , TotalSteps(0)
		  , AsyncTask(nullptr)
	{
	}

	/** 소멸자에서 스레드 정리 */
	virtual void BeginDestroy() override;

	/** WFC3D 시각화를 동기적으로 실행합니다 (메인 스레드) */
	UFUNCTION(BlueprintCallable, Category = "WFCVisualization")
	FWFC3DVisualizeResult Execute(const FWFC3DVisualizeContext& Context);

	/** WFC3D 시각화를 비동기적으로 실행합니다 (백그라운드 스레드) */
	UFUNCTION(BlueprintCallable, Category = "WFCVisualization")
	void ExecuteAsync(const FWFC3DVisualizeContext& Context);

	/** 실행 중인 시각화를 취소합니다 */
	UFUNCTION(BlueprintCallable, Category = "WFCVisualization")
	void CancelExecution();

	/** 시각화 실행 상태를 초기화합니다 */
	UFUNCTION(BlueprintCallable, Category = "WFCVisualization")
	void ResetExecutionState();

	/** 현재 시각화가 실행 중인지 확인합니다 */
	UFUNCTION(BlueprintPure, Category = "WFCVisualization")
	bool IsRunning() const { return bIsRunning; }

	/** 현재 시각화가 취소되었는지 확인합니다 */
	UFUNCTION(BlueprintPure, Category = "WFCVisualization")
	bool IsCancelled() const { return bIsCancelled; }

	/** 현재 시각화가 완료되었는지 확인합니다 */
	UFUNCTION(BlueprintPure, Category = "WFCVisualization")
	bool IsComplete() const { return bIsComplete; }

	/** 현재 진행률을 반환합니다 (0.0 ~ 1.0) */
	UFUNCTION(BlueprintPure, Category = "WFCVisualization")
	float GetProgress() const;

	/** 시각화 컴포넌트를 반환합니다 */
	UFUNCTION(BlueprintPure, Category = "WFCVisualization")
	USceneComponent* GetRootComponent() const { return RootComponent; }
	
	/** 내부 실행 함수 (스레드 안전) */
	FWFC3DVisualizeResult ExecuteInternal(const FWFC3DVisualizeContext& Context);

	/** 비동기 작업 상태를 확인하고 결과를 처리합니다 */
	UFUNCTION(BlueprintCallable, Category = "WFCVisualization")
	void CheckAsyncTaskCompletion(const FWFC3DVisualizeContext& Context);

	/** 시각화 완료 시 호출되는 델리게이트 */
	UPROPERTY(BlueprintAssignable, Category = "WFCVisualization")
	FOnWFC3DVisualizationCompleted OnVisualizationCompleted;

	/** 시각화 취소 시 호출되는 델리게이트 */
	UPROPERTY(BlueprintAssignable, Category = "WFCVisualization")
	FOnWFC3DVisualizationCancelled OnVisualizationCancelled;

	/** 시각화 진행률 업데이트 시 호출되는 델리게이트 */
	UPROPERTY(BlueprintAssignable, Category = "WFCVisualization")
	FOnWFC3DVisualizationProgress OnVisualizationProgress;

protected:
	/** 시각화가 현재 실행 중인지 나타내는 플래그 */
	UPROPERTY(BlueprintReadOnly, Category = "WFCVisualization")
	bool bIsRunning;

	/** 시각화 실행이 취소되었는지 나타내는 플래그 */
	UPROPERTY(BlueprintReadOnly, Category = "WFCVisualization")
	bool bIsCancelled;

	/** 시각화 실행이 완료되었는지 나타내는 플래그 */
	UPROPERTY(BlueprintReadOnly, Category = "WFCVisualization")
	bool bIsComplete;

	/** 현재 진행 단계 */
	UPROPERTY(BlueprintReadOnly, Category = "WFCVisualization")
	int32 CurrentStep;

	/** 전체 단계 수 */
	UPROPERTY(BlueprintReadOnly, Category = "WFCVisualization")
	int32 TotalSteps;

	/** 스레드 안전한 실행 상태 */
	std::atomic<bool> bIsRunningAtomic;
	std::atomic<bool> bIsCancelledAtomic;
	std::atomic<bool> bIsCompleteAtomic;
	std::atomic<int32> CurrentStepAtomic;
	std::atomic<int32> TotalStepsAtomic;

	/** 비동기 태스크 인스턴스 */
	TUniquePtr<FAsyncTask<FWFC3DVisualizeAsyncTask>> AsyncTask;

	/** 크리티컬 섹션 (스레드 동기화용) */
	mutable FCriticalSection CriticalSection;

	/** 비동기 작업 완료 체크용 타이머 핸들 */
	FTimerHandle AsyncCheckTimerHandle;

	/** 생성된 메시 컴포넌트들을 저장하는 배열 */
	UPROPERTY()
	TArray<UStaticMeshComponent*> SpawnedMeshComponents;

	/** 타일 크기 (월드 단위) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WFCVisualization")
	float TileSize = 100.0f;

	/** 기본 바이옴 이름 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WFCVisualization")
	FString DefaultBiomeName = "None";

	/** 사용할 타일 변형 인덱스 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WFCVisualization")
	int32 DefaultVariantIndex = 0;

private:

	/** 단일 셀의 시각화 데이터를 준비하는 함수 */
	bool SetTileVisualInfo(FWFC3DCell& Cell, const UWFC3DModelDataAsset* ModelData, const FRandomStream* RandomStream);

	/** 준비된 데이터로 실제 메시를 생성하는 함수 (메인 스레드 전용) */
	void CreateMeshesFromData(UWorld* World, UWFC3DGrid* Grid, const UWFC3DModelDataAsset* ModelData);

	/** 회전값을 FRotator로 변환하는 함수 */
	static FRotator GetRotationFromRotationStep(int32 RotationStep);

	/** 스폰된 메시 컴포넌트들을 정리하는 함수 */
	void CleanupSpawnedMeshes();
	
	/** 시각화를 위한 루트 컴포넌트 (메시들의 부모) */
	UPROPERTY()
	USceneComponent* RootComponent = nullptr;
	
};