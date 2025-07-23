// Copyright Epic Games, Inc. All Rights Reserved.

#include "WFC/Example/WFC3DAsyncExample.h"
#include "Engine/Engine.h"
#include "Async/Async.h"
#include "Async/TaskGraphInterfaces.h"
#include "WFC/Data/WFC3DGrid.h"

AWFC3DAsyncExample::AWFC3DAsyncExample()
{
	PrimaryActorTick.bCanEverTick = false;

	// WFC 알고리즘 인스턴스 생성
	WFCAlgorithm = CreateDefaultSubobject<UWFC3DAlgorithm>(TEXT("WFCAlgorithm"));
}

void AWFC3DAsyncExample::BeginPlay()
{
	Super::BeginPlay();

	UE_LOG(LogTemp, Error, TEXT("Begin Play: WFC3DAsyncExample 시작"));

	// 델리게이트 바인딩
	if (WFCAlgorithm)
	{
		UE_LOG(LogTemp, Display, TEXT("Add Delegates to WFCAlgorithm"));
		WFCAlgorithm->OnAlgorithmCompleted.AddDynamic(this, &AWFC3DAsyncExample::OnAlgorithmCompleted);
		WFCAlgorithm->OnAlgorithmCancelled.AddDynamic(this, &AWFC3DAsyncExample::OnAlgorithmCancelled);
		WFCAlgorithm->OnAlgorithmProgress.AddDynamic(this, &AWFC3DAsyncExample::OnAlgorithmProgress);
	}

	// 알고리즘 컨텍스트 설정 Grid 크기 (5,5,5)
	AlgorithmContext.Grid = NewObject<UWFC3DGrid>();

	ExecuteAsync();
}

void AWFC3DAsyncExample::ExecuteSync()
{
	if (!WFCAlgorithm)
	{
		UE_LOG(LogTemp, Error, TEXT("WFCAlgorithm is null"));
		return;
	}

	UE_LOG(LogTemp, Log, TEXT("=== 동기 실행 시작 (메인 스레드) ==="));

	// 메인 스레드에서 동기적으로 실행
	FWFC3DResult Result = WFCAlgorithm->Execute(AlgorithmContext);

	if (Result.bSuccess)
	{
		UE_LOG(LogTemp, Log, TEXT("동기 실행 완료! 결과: 성공"));
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("동기 실행 완료! 결과: 실패"));
	}
}

void AWFC3DAsyncExample::ExecuteAsync()
{
	if (!WFCAlgorithm)
	{
		UE_LOG(LogTemp, Error, TEXT("WFCAlgorithm is null"));
		return;
	}

	UE_LOG(LogTemp, Log, TEXT("=== 비동기 실행 시작 (FAsyncTask 사용) ==="));

	// 테스트용 Grid 생성 및 초기화
	UWFC3DGrid* TestGrid = NewObject<UWFC3DGrid>();
	TestGrid->InitializeGrid({TestGridSize.X, TestGridSize.Y, TestGridSize.Z}, TestModelData); // 555 그리드로 테스트

	// 컨텍스트 생성
	FWFC3DAlgorithmContext TestContext(TestGrid, TestModelData);
	AlgorithmContext = TestContext;
	
	UE_LOG(LogTemp, Warning, TEXT("🧪 테스트 컨텍스트 생성됨"));
	UE_LOG(LogTemp, Warning, TEXT("Grid: %s"), TestGrid ? TEXT("Valid") : TEXT("Invalid"));
	UE_LOG(LogTemp, Warning, TEXT("Grid Dimension: %s"), *TestGrid->GetDimension().ToString());
	UE_LOG(LogTemp, Warning, TEXT("Grid Remaining Cells: %d"), TestGrid->GetRemainingCells());

	// 진행률 표시 타이머 시작
	GetWorldTimerManager().SetTimer(
		ProgressTimerHandle,
		this,
		&AWFC3DAsyncExample::ShowProgress,
		0.5f, // 0.5초마다
		true
	);
	
	// 비동기 실행
	WFCAlgorithm->ExecuteAsync(AlgorithmContext);
}

void AWFC3DAsyncExample::ExecuteWithTaskGraph()
{
	if (!WFCAlgorithm)
	{
		UE_LOG(LogTemp, Error, TEXT("WFCAlgorithm is null"));
		return;
	}

	UE_LOG(LogTemp, Log, TEXT("=== Task Graph를 사용한 비동기 실행 시작 ==="));

	// Task Graph를 사용한 비동기 실행
	AsyncTask(ENamedThreads::AnyBackgroundThreadNormalTask, [this]()
	{
		// 백그라운드 스레드에서 실행
		FWFC3DResult Result = WFCAlgorithm->ExecuteInternal(AlgorithmContext);

		// 메인 스레드에서 결과 처리
		AsyncTask(ENamedThreads::GameThread, [this, Result]()
		{
			OnTaskGraphCompleted(Result);
		});
	});
}

void AWFC3DAsyncExample::ExecuteWithThreadPool()
{
	if (!WFCAlgorithm)
	{
		UE_LOG(LogTemp, Error, TEXT("WFCAlgorithm is null"));
		return;
	}

	UE_LOG(LogTemp, Log, TEXT("=== Thread Pool을 사용한 비동기 실행 시작 ==="));

	// Thread Pool을 사용한 비동기 실행
	Async(EAsyncExecution::ThreadPool, [this]()
	{
		// Thread Pool에서 실행
		FWFC3DResult Result = WFCAlgorithm->ExecuteInternal(AlgorithmContext);

		// 메인 스레드에서 결과 처리
		Async(EAsyncExecution::TaskGraphMainThread, [this, Result]()
		{
			OnThreadPoolCompleted(Result);
		});
	});
}

void AWFC3DAsyncExample::CancelExecution()
{
	if (!WFCAlgorithm)
	{
		UE_LOG(LogTemp, Error, TEXT("WFCAlgorithm is null"));
		return;
	}

	UE_LOG(LogTemp, Warning, TEXT("=== 실행 취소 요청 ==="));
	WFCAlgorithm->CancelExecution();
}

void AWFC3DAsyncExample::CheckStatus()
{
	if (!WFCAlgorithm)
	{
		UE_LOG(LogTemp, Error, TEXT("WFCAlgorithm is null"));
		return;
	}

	UE_LOG(LogTemp, Log, TEXT("=== 현재 상태 ==="));
	UE_LOG(LogTemp, Log, TEXT("실행 중: %s"), WFCAlgorithm->IsRunning() ? TEXT("예") : TEXT("아니오"));
	UE_LOG(LogTemp, Log, TEXT("취소됨: %s"), WFCAlgorithm->IsCancelled() ? TEXT("예") : TEXT("아니오"));
	UE_LOG(LogTemp, Log, TEXT("완료됨: %s"), WFCAlgorithm->IsComplete() ? TEXT("예") : TEXT("아니오"));
	UE_LOG(LogTemp, Log, TEXT("진행률: %.1f%%"), WFCAlgorithm->GetProgress() * 100.0f);
}

void AWFC3DAsyncExample::OnAlgorithmCompleted(const FWFC3DResult& Result)
{
	// 진행률 타이머 정리
	GetWorldTimerManager().ClearTimer(ProgressTimerHandle);

	UE_LOG(LogTemp, Log, TEXT("=== 알고리즘 완료! ==="));
	UE_LOG(LogTemp, Log, TEXT("성공: %s"), Result.bSuccess ? TEXT("예") : TEXT("아니오"));
	UE_LOG(LogTemp, Log, TEXT("Collapse 결과 수: %d"), Result.CollapseResults.Num());
	UE_LOG(LogTemp, Log, TEXT("Propagation 결과 수: %d"), Result.PropagationResults.Num());
	
	// 화면에 메시지 표시
	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(
			-1, 5.0f, FColor::Green,
			FString::Printf(TEXT("WFC 알고리즘 완료! 성공: %s"),
			                Result.bSuccess ? TEXT("예") : TEXT("아니오"))
		);
	}
}

void AWFC3DAsyncExample::OnAlgorithmCancelled()
{
	// 진행률 타이머 정리
	GetWorldTimerManager().ClearTimer(ProgressTimerHandle);

	UE_LOG(LogTemp, Warning, TEXT("=== 알고리즘 취소됨! ==="));

	// 화면에 메시지 표시
	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Yellow, TEXT("WFC 알고리즘이 취소되었습니다!"));
	}
}

void AWFC3DAsyncExample::OnAlgorithmProgress(int32 CurrentStep, int32 TotalSteps)
{
	float Progress = TotalSteps > 0 ? (float)CurrentStep / (float)TotalSteps * 100.0f : 0.0f;

	UE_LOG(LogTemp, Log, TEXT("진행률: %d/%d (%.1f%%)"), CurrentStep, TotalSteps, Progress);

	// 화면에 진행률 표시
	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(
			1, 1.0f, FColor::Blue,
			FString::Printf(TEXT("진행률: %d/%d (%.1f%%)"), CurrentStep, TotalSteps, Progress)
		);
	}
}

void AWFC3DAsyncExample::ShowProgress()
{
	if (WFCAlgorithm)
	{
		float Progress = WFCAlgorithm->GetProgress() * 100.0f;

		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(
				2, 1.0f, FColor::Cyan,
				FString::Printf(TEXT("현재 진행률: %.1f%%"), Progress)
			);
		}
	}
}

void AWFC3DAsyncExample::OnTaskGraphCompleted(FWFC3DResult Result)
{
	UE_LOG(LogTemp, Log, TEXT("=== Task Graph 실행 완료! ==="));
	UE_LOG(LogTemp, Log, TEXT("성공: %s"), Result.bSuccess ? TEXT("예") : TEXT("아니오"));

	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(
			-1, 5.0f, FColor::Purple,
			FString::Printf(TEXT("Task Graph 완료! 성공: %s"),
			                Result.bSuccess ? TEXT("예") : TEXT("아니오"))
		);
	}
}

void AWFC3DAsyncExample::OnThreadPoolCompleted(FWFC3DResult Result)
{
	UE_LOG(LogTemp, Log, TEXT("=== Thread Pool 실행 완료! ==="));
	UE_LOG(LogTemp, Log, TEXT("성공: %s"), Result.bSuccess ? TEXT("예") : TEXT("아니오"));

	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(
			-1, 5.0f, FColor::Orange,
			FString::Printf(TEXT("Thread Pool 완료! 성공: %s"),
			                Result.bSuccess ? TEXT("예") : TEXT("아니오"))
		);
	}
}
