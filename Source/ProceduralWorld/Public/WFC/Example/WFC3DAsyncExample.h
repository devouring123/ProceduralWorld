// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "WFC/Algorithm/WFC3DAlgorithm.h"
#include "Engine/TimerHandle.h"
#include "WFC3DAsyncExample.generated.h"

/**
 * WFC3D 알고리즘 비동기 실행 예시 클래스
 * 다양한 스레딩 방법을 보여줍니다.
 */
UCLASS(BlueprintType, Blueprintable)
class PROCEDURALWORLD_API AWFC3DAsyncExample : public AActor
{
	GENERATED_BODY()
	
public:	
	AWFC3DAsyncExample();

protected:
	virtual void BeginPlay() override;

public:	
	/** 메인 스레드에서 동기적으로 실행 */
	UFUNCTION(BlueprintCallable, Category = "WFC Example")
	void ExecuteSync();

	/** 백그라운드 스레드에서 비동기적으로 실행 */
	UFUNCTION(BlueprintCallable, Category = "WFC Example")
	void ExecuteAsync();

	/** Task Graph를 사용한 비동기 실행 */
	UFUNCTION(BlueprintCallable, Category = "WFC Example")
	void ExecuteWithTaskGraph();

	/** Thread Pool을 사용한 비동기 실행 */
	UFUNCTION(BlueprintCallable, Category = "WFC Example")
	void ExecuteWithThreadPool();

	/** 실행 취소 */
	UFUNCTION(BlueprintCallable, Category = "WFC Example")
	void CancelExecution();

	/** 상태 확인 */
	UFUNCTION(BlueprintCallable, Category = "WFC Example")
	void CheckStatus();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WFC Example")
	UWFC3DModelDataAsset* TestModelData;
	
protected:
	/** WFC3D 알고리즘 인스턴스 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "WFC")
	UWFC3DAlgorithm* WFCAlgorithm;

	/** 알고리즘 컨텍스트 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WFC")
	FWFC3DAlgorithmContext AlgorithmContext;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WFC")
	FIntVector TestGridSize = FIntVector(5, 5, 5);

	bool bIsSuccess = false;

	int32 TryCount = 0;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WFC")
	int32 MaxTryCount = 100;
	
	/** 진행률 표시용 타이머 */
	FTimerHandle ProgressTimerHandle;

	/** 알고리즘 완료 콜백 */
	UFUNCTION()
	void OnAlgorithmCompleted(const FWFC3DResult& Result);

	/** 알고리즘 취소 콜백 */
	UFUNCTION()
	void OnAlgorithmCancelled();

	/** 알고리즘 진행률 콜백 */
	UFUNCTION()
	void OnAlgorithmProgress(int32 CurrentStep, int32 TotalSteps);

	/** 진행률 표시 함수 */
	void ShowProgress();

	/** Task Graph 완료 콜백 */
	void OnTaskGraphCompleted(FWFC3DResult Result);

	/** Thread Pool 완료 콜백 */
	void OnThreadPoolCompleted(FWFC3DResult Result);
};
