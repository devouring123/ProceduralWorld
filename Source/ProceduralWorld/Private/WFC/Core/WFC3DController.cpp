// Fill out your copyright notice in the Description page of Project Settings.

#include "WFC/Core/WFC3DController.h"
#include "WFC/Algorithm/WFC3DAlgorithm.h"
#include "WFC/Visualization/WFC3DVisualizer.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "Components/SceneComponent.h"
#include "Async/Async.h"

UWFC3DController::UWFC3DController()
	: bIsRunning(false)
	, bIsCancelled(false)
	, bIsComplete(false)
	, CurrentPhaseName(TEXT("Idle"))
	, GeneratedGrid(nullptr)
	, Algorithm(nullptr)
	, Visualizer(nullptr)
	, bIsRunningAtomic(false)
	, bIsCancelledAtomic(false)
	, bIsCompleteAtomic(false)
	, CurrentStepAtomic(0)
	, TotalStepsAtomic(0)
{
	// 알고리즘 인스턴스 생성 - CreateDefaultSubobject 사용
	Algorithm = CreateDefaultSubobject<UWFC3DAlgorithm>(TEXT("Algorithm"));
	
	// 시각화 인스턴스 생성 - CreateDefaultSubobject 사용
	Visualizer = CreateDefaultSubobject<UWFC3DVisualizer>(TEXT("Visualizer"));
	
	// 델리게이트 바인딩
	BindAlgorithmDelegates();
	BindVisualizationDelegates();
}

void UWFC3DController::BeginDestroy()
{
	UE_LOG(LogTemp, Warning, TEXT("WFC3DController::BeginDestroy() called - Cancelling running tasks"));
	
	// 실행 중인 작업 취소
	if (bIsRunning)
	{
		CancelExecution();
		
		// 백그라운드 작업이 완료될 때까지 잠시 대기 (최대 1초)
		double StartTime = FPlatformTime::Seconds();
		const double MaxWaitTime = 1.0; // 1초 최대 대기
		
		while (bIsRunningAtomic.load() && (FPlatformTime::Seconds() - StartTime) < MaxWaitTime)
		{
			// 짧은 대기를 통해 백그라운드 작업이 취소 신호를 받고 종료할 시간을 줌
			FPlatformProcess::Sleep(0.01f); // 10ms씩 대기
		}
		
		if (bIsRunningAtomic.load())
		{
			UE_LOG(LogTemp, Error, TEXT("WFC3DController: Background task did not complete within timeout during destruction"));
		}
		else
		{
			UE_LOG(LogTemp, Log, TEXT("WFC3DController: Background task successfully cancelled and completed"));
		}
	}
	
	// 델리게이트 언바인딩
	UnbindDelegates();
	
	// 타이머 정리
	UWorld* World = GEngine ? GEngine->GetWorldFromContextObject(this, EGetWorldErrorMode::LogAndReturnNull) : nullptr;
	if (World && AsyncCheckTimerHandle.IsValid())
	{
		World->GetTimerManager().ClearTimer(AsyncCheckTimerHandle);
		UE_LOG(LogTemp, Log, TEXT("WFC3DController: Async check timer cleared"));
	}
	
	// GeneratedGrid 정리
	if (GeneratedGrid && IsValid(GeneratedGrid))
	{
		GeneratedGrid->ConditionalBeginDestroy();
		GeneratedGrid = nullptr;
	}
	
	// Algorithm과 Visualizer는 CreateDefaultSubobject로 생성되었으므로 자동으로 정리됨
	
	UE_LOG(LogTemp, Log, TEXT("WFC3DController::BeginDestroy() completed"));
	Super::BeginDestroy();
}

FWFC3DExecutionResult UWFC3DController::Execute(const FWFC3DExecutionContext& Context)
{
	return ExecuteInternal(Context);
}

void UWFC3DController::ExecuteAsync(const FWFC3DExecutionContext& Context)
{
	// 이미 실행 중이면 무시
	if (bIsRunning)
	{
		UE_LOG(LogTemp, Warning, TEXT("WFC3DController is already running"));
		return;
	}
	
	// 상태 초기화
	ResetExecutionState();
	
	// 비동기 실행
	AsyncTask(ENamedThreads::AnyBackgroundHiPriTask, [this, Context]()
	{
		// 백그라운드 스레드 시작 로깅
		UE_LOG(LogTemp, Log, TEXT("WFC3D Background task started"));
		
		FWFC3DExecutionResult Result = ExecuteInternal(Context);
		
		// 백그라운드 스레드 완료 로깅
		UE_LOG(LogTemp, Log, TEXT("WFC3D Background task completed, scheduling game thread callback"));
		
		// 메인 스레드에서 완료 처리
		AsyncTask(ENamedThreads::GameThread, [this, Result, Context]()
		{
			// 오브젝트가 아직 유효한지 확인
			if (IsValid(this) && !IsUnreachable())
			{
				CheckAsyncCompletion(Context);
			}
			else
			{
				UE_LOG(LogTemp, Warning, TEXT("WFC3DController is no longer valid, skipping async completion"));
			}
		});
	});
	
	// 비동기 체크 타이머 설정
	if (UWorld* World = GEngine ? GEngine->GetWorldFromContextObject(this, EGetWorldErrorMode::LogAndReturnNull) : nullptr)
	{
		World->GetTimerManager().SetTimer(
			AsyncCheckTimerHandle,
			FTimerDelegate::CreateLambda([this, Context]()
			{
				// 오브젝트가 아직 유효한지 확인
				if (IsValid(this) && !IsUnreachable())
				{
					CheckAsyncCompletion(Context);
				}
				else
				{
					UE_LOG(LogTemp, Warning, TEXT("WFC3DController is no longer valid, stopping timer"));
					// 타이머를 정리하려고 시도하지만, World가 유효하지 않을 수 있음
				}
			}),
			0.1f, // 0.1초마다 체크
			true
		);
	}
}

void UWFC3DController::CancelExecution()
{
	if (!bIsRunning)
	{
		return;
	}
	
	FScopeLock Lock(&CriticalSection);
	
	bIsCancelled = true;
	bIsCancelledAtomic = true;
	
	// 알고리즘 취소
	if (Algorithm)
	{
		Algorithm->CancelExecution();
	}
	
	// 시각화 취소
	if (Visualizer)
	{
		Visualizer->CancelExecution();
	}
	
	SetCurrentPhase(TEXT("Cancelled"));
	
	// 취소 델리게이트 호출
	OnExecutionCancelled.Broadcast();
}

void UWFC3DController::ResetExecutionState()
{
	FScopeLock Lock(&CriticalSection);
	
	bIsRunning = false;
	bIsCancelled = false;
	bIsComplete = false;
	CurrentPhaseName = TEXT("Idle");
	
	// Atomic 변수들도 초기화
	bIsRunningAtomic = false;
	bIsCancelledAtomic = false;
	bIsCompleteAtomic = false;
	CurrentStepAtomic = 0;
	TotalStepsAtomic = 0;
	
	// 타이머 정리
	UWorld* World = GEngine ? GEngine->GetWorldFromContextObject(this, EGetWorldErrorMode::LogAndReturnNull) : nullptr;
	if (World && AsyncCheckTimerHandle.IsValid())
	{
		World->GetTimerManager().ClearTimer(AsyncCheckTimerHandle);
	}
	
	// 알고리즘 상태 초기화
	if (Algorithm)
	{
		Algorithm->ResetExecutionState();
	}
	
	// 시각화 상태 초기화
	if (Visualizer)
	{
		Visualizer->ResetExecutionState();
	}
}

float UWFC3DController::GetProgress() const
{
	int32 Total = TotalStepsAtomic.load();
	if (Total <= 0)
	{
		return 0.0f;
	}
	
	int32 Current = CurrentStepAtomic.load();
	return FMath::Clamp(static_cast<float>(Current) / static_cast<float>(Total), 0.0f, 1.0f);
}

USceneComponent* UWFC3DController::GetVisualizationComponent() const
{
	if (Visualizer)
	{
		return Visualizer->GetRootComponent();
	}
	return nullptr;
}

FWFC3DExecutionResult UWFC3DController::ExecuteInternal(const FWFC3DExecutionContext& Context)
{
	UE_LOG(LogTemp, Log, TEXT("ExecuteInternal started"));
	
	// 시작하기 전에 이미 취소되었는지 확인
	if (bIsCancelledAtomic.load())
	{
		UE_LOG(LogTemp, Warning, TEXT("ExecuteInternal: Already cancelled before start"));
		FWFC3DExecutionResult Result;
		Result.bSuccess = false;
		Result.ErrorMessage = TEXT("Execution was cancelled before start");
		return Result;
	}
	
	FScopeLock Lock(&CriticalSection);
	
	FWFC3DExecutionResult Result;
	
	// 실행 상태 초기화
	bIsRunning = true;
	bIsCancelled = false;
	bIsComplete = false;
	bIsRunningAtomic = true;
	bIsCancelledAtomic = false;
	bIsCompleteAtomic = false;
	
	// 입력 검증
	if (!Context.ModelData)
	{
		UE_LOG(LogTemp, Error, TEXT("ModelData is null"));
		Result.bSuccess = false;
		Result.ErrorMessage = TEXT("ModelData is null");
		bIsRunning = false;
		bIsRunningAtomic = false;
		return Result;
	}
	
	SetCurrentPhase(TEXT("Starting"));

	// 1단계: 알고리즘 실행
	Result.AlgorithmResult = ExecuteAlgorithm(Context);
	Result.bSuccess = Result.AlgorithmResult.bSuccess;
	Result.ErrorMessage = Result.AlgorithmResult.ErrorMessage;
	
	UE_LOG(LogTemp, Log, TEXT("Algorithm phase result - Success: %s, Error: %s"), 
		Result.bSuccess ? TEXT("YES") : TEXT("NO"), 
		*Result.ErrorMessage);
	
	if (!Result.bSuccess || bIsCancelledAtomic.load())
	{
		UE_LOG(LogTemp, Error, TEXT("Algorithm failed or cancelled, stopping execution"));
		bIsRunning = false;
		bIsRunningAtomic = false;
		return Result;
	}
	
	// 2단계: 시각화 실행 (활성화된 경우)
	UE_LOG(LogTemp, Log, TEXT("Checking if visualization is enabled: %s"), Context.bEnableVisualization ? TEXT("YES") : TEXT("NO"));
	if (Context.bEnableVisualization)
	{
		UE_LOG(LogTemp, Log, TEXT("Starting visualization phase..."));
		Result.VisualizeResult = ExecuteVisualization(Context);
		if (!Result.VisualizeResult.bSuccess)
		{
			UE_LOG(LogTemp, Warning, TEXT("Visualization failed, but algorithm succeeded"));
			Result.bSuccess = false; // 시각화 실패 시 전체 결과도 실패로 처리
			Result.ErrorMessage = Result.VisualizeResult.ErrorMessage;
		}
		else
		{
			UE_LOG(LogTemp, Log, TEXT("Visualization phase completed successfully"));
		}
	}
	else
	{
		UE_LOG(LogTemp, Log, TEXT("Visualization is disabled, skipping visualization phase"));
	}
	
	// 완료 처리
	UE_LOG(LogTemp, Log, TEXT("ExecuteInternal completing with success: %s"), Result.bSuccess ? TEXT("YES") : TEXT("NO"));
	bIsComplete = true;
	bIsRunning = false;
	bIsCompleteAtomic = true;
	bIsRunningAtomic = false;
	
	SetCurrentPhase(TEXT("Completed"), 1.0f);
	UE_LOG(LogTemp, Log, TEXT("ExecuteInternal completed successfully"));
	
	return Result;
}

FWFC3DAlgorithmResult UWFC3DController::ExecuteAlgorithm(const FWFC3DExecutionContext& Context)
{
	UE_LOG(LogTemp, Log, TEXT("ExecuteAlgorithm started with MaxRetryCount: %d"), Context.MaxRetryCount);
	SetCurrentPhase(TEXT("Algorithm"), 0.0f);
	
	if (!Algorithm)
	{
		UE_LOG(LogTemp, Error, TEXT("Algorithm is null!"));
		FWFC3DAlgorithmResult Result;
		Result.bSuccess = false;
		Result.ErrorMessage = TEXT("Algorithm is null");
		return Result;
	}

	FWFC3DAlgorithmResult FinalResult;
	FinalResult.bSuccess = false;
	
	// 재시도 로직 - MaxRetryCount만큼 시도
	for (int32 AttemptCount = 1; AttemptCount <= Context.MaxRetryCount; AttemptCount++)
	{
		// 각 시도 시작 전에 취소 요청 확인
		if (bIsCancelledAtomic.load())
		{
			UE_LOG(LogTemp, Warning, TEXT("=== WFC Algorithm CANCELLED before attempt %d ==="), AttemptCount);
			FinalResult.bSuccess = false;
			FinalResult.ErrorMessage = TEXT("Algorithm execution was cancelled");
			break;
		}
		
		UE_LOG(LogTemp, Warning, TEXT("=== WFC Algorithm Attempt %d/%d ==="), AttemptCount, Context.MaxRetryCount);
		SetCurrentPhase(FString::Printf(TEXT("Algorithm (Attempt %d/%d)"), AttemptCount, Context.MaxRetryCount), 0.0f);
		
		UE_LOG(LogTemp, Log, TEXT("Creating Grid for attempt %d..."), AttemptCount);
		// 그리드 생성 - 런타임에서는 NewObject 사용 가능 (Transient로 생성하여 GC 문제 방지)
		UWFC3DGrid* Grid = NewObject<UWFC3DGrid>(GetTransientPackage());
		if (!Grid)
		{
			UE_LOG(LogTemp, Error, TEXT("Failed to create Grid for attempt %d!"), AttemptCount);
			FinalResult.ErrorMessage = TEXT("Failed to create Grid");
			continue; // 다음 시도로 넘어감
		}
		
		UE_LOG(LogTemp, Log, TEXT("Grid created successfully for attempt %d, initializing..."), AttemptCount);
		// 그리드 초기화
		Grid->InitializeGrid(Context.GridDimension, Context.ModelData);
		UE_LOG(LogTemp, Log, TEXT("Grid initialized successfully for attempt %d"), AttemptCount);
		
		UE_LOG(LogTemp, Log, TEXT("Setting up Algorithm context for attempt %d..."), AttemptCount);
		// 알고리즘 컨텍스트 구성
		FWFC3DAlgorithmContext AlgorithmContext;
		AlgorithmContext.Grid = Grid;
		AlgorithmContext.ModelData = Context.ModelData;
		
		// 각 시도마다 다른 랜덤 시드를 사용하여 성공 확률 증가
		// 원래 시드에 시도 횟수를 더해서 변형
		int32 CurrentSeed = Context.RandomSeed;
		if (CurrentSeed == 0)
		{
			CurrentSeed = FRandomStream(Context.RandomSeed + AttemptCount).FRandRange(0,123456789); 
		}
		else
		{
			CurrentSeed = FRandomStream(Context.RandomSeed + AttemptCount).FRandRange(0,123456789); 
		}
		UE_LOG(LogTemp, Log, TEXT("Using RandomSeed: %d for attempt %d (Base: %d)"), CurrentSeed, AttemptCount, Context.RandomSeed);
		
		// TODO: 알고리즘에 랜덤 시드를 전달하는 방법이 필요하면 추가
		
		UE_LOG(LogTemp, Log, TEXT("Executing Algorithm for attempt %d..."), AttemptCount);
		// 알고리즘 실행
		FWFC3DAlgorithmResult Result = Algorithm->Execute(AlgorithmContext);
		UE_LOG(LogTemp, Log, TEXT("Algorithm attempt %d completed with result: %s"), 
			AttemptCount, Result.bSuccess ? TEXT("SUCCESS") : TEXT("FAILED"));
		
		// 알고리즘 실행 후 취소 요청 확인
		if (bIsCancelledAtomic.load())
		{
			UE_LOG(LogTemp, Warning, TEXT("=== WFC Algorithm CANCELLED after attempt %d ==="), AttemptCount);
			FinalResult.bSuccess = false;
			FinalResult.ErrorMessage = TEXT("Algorithm execution was cancelled");
			break;
		}
		
		if (Result.bSuccess)
		{
			UE_LOG(LogTemp, Warning, TEXT("=== WFC Algorithm SUCCEEDED on attempt %d/%d ==="), AttemptCount, Context.MaxRetryCount);
			GeneratedGrid = Grid; // 이미 Algorithm이 Grid를 수정했으므로 그대로 사용
			SetCurrentPhase(TEXT("Algorithm"), 1.0f);
			UE_LOG(LogTemp, Log, TEXT("Algorithm phase completed successfully on attempt %d"), AttemptCount);
			
			// 성공한 결과를 반환
			FinalResult = Result;
			break; // 성공했으므로 루프 종료
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("Algorithm attempt %d failed with error: %s"), AttemptCount, *Result.ErrorMessage);
			FinalResult = Result; // 실패한 결과를 저장 (마지막 실패 이유 보존)
			
			// 마지막 시도가 아니면 계속
			if (AttemptCount < Context.MaxRetryCount)
			{
				UE_LOG(LogTemp, Log, TEXT("Retrying algorithm... (%d attempts remaining)"), Context.MaxRetryCount - AttemptCount);
				
				// 다음 시도 전에 취소 요청 재확인
				if (bIsCancelledAtomic.load())
				{
					UE_LOG(LogTemp, Warning, TEXT("=== WFC Algorithm CANCELLED before retry ==="));
					FinalResult.bSuccess = false;
					FinalResult.ErrorMessage = TEXT("Algorithm execution was cancelled");
					break;
				}
			}
		}
	}
	
	// 모든 시도가 실패한 경우
	if (!FinalResult.bSuccess)
	{
		UE_LOG(LogTemp, Error, TEXT("=== WFC Algorithm FAILED after %d attempts ==="), Context.MaxRetryCount);
		SetCurrentPhase(TEXT("Algorithm Failed"));
		UE_LOG(LogTemp, Error, TEXT("WFC3D Algorithm failed after %d attempts. Last error: %s"), Context.MaxRetryCount, *FinalResult.ErrorMessage);
	}
	
	return FinalResult;
}

FWFC3DVisualizeResult UWFC3DController::ExecuteVisualization(const FWFC3DExecutionContext& Context)
{
	UE_LOG(LogTemp, Log, TEXT("ExecuteVisualization started"));
	SetCurrentPhase(TEXT("Visualization"), 0.0f);
	
	if (!Visualizer)
	{
		UE_LOG(LogTemp, Error, TEXT("Visualizer is null!"));
		FWFC3DVisualizeResult Result;
		Result.bSuccess = false;
		Result.ErrorMessage = TEXT("Visualizer is null");
		return Result;
	}
	
	if (!GeneratedGrid)
	{
		UE_LOG(LogTemp, Error, TEXT("GeneratedGrid is null!"));
		FWFC3DVisualizeResult Result;
		Result.bSuccess = false;
		Result.ErrorMessage = TEXT("GeneratedGrid is null");
		return Result;
	}
	
	UE_LOG(LogTemp, Log, TEXT("Visualizer and GeneratedGrid are valid"));
	
	UE_LOG(LogTemp, Log, TEXT("Setting up Visualization context..."));
	// 시각화 컨텍스트 구성
	FWFC3DVisualizeContext VisualizationContext;
	VisualizationContext.Grid = GeneratedGrid;
	VisualizationContext.ModelData = Context.ModelData;
	
	UE_LOG(LogTemp, Log, TEXT("Creating RandomStream for visualization..."));
	// RandomStream 생성 및 할당
	FRandomStream VisualizationRandomStream(Context.RandomSeed);
	VisualizationContext.RandomStream = &VisualizationRandomStream;
	
	UE_LOG(LogTemp, Log, TEXT("Executing Visualization..."));
	// 시각화 실행
	FWFC3DVisualizeResult Result = Visualizer->Execute(VisualizationContext);
	UE_LOG(LogTemp, Log, TEXT("Visualization execution completed with result: %s"), Result.bSuccess ? TEXT("SUCCESS") : TEXT("FAILED"));
	
	if (Result.bSuccess)
	{
		UE_LOG(LogTemp, Log, TEXT("Visualization succeeded"));
		SetCurrentPhase(TEXT("Visualization"), 1.0f);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Visualization failed with error: %s"), *Result.ErrorMessage);
		SetCurrentPhase(TEXT("Visualization Failed"));
		UE_LOG(LogTemp, Error, TEXT("WFC3D Visualization failed: %s"), *Result.ErrorMessage);
	}
	
	return Result;
}

void UWFC3DController::CheckAsyncCompletion(const FWFC3DExecutionContext& Context)
{
	if (!bIsRunningAtomic.load())
	{
		// 타이머 정리
		UWorld* World = GEngine ? GEngine->GetWorldFromContextObject(this, EGetWorldErrorMode::LogAndReturnNull) : nullptr;
		if (World && AsyncCheckTimerHandle.IsValid())
		{
			World->GetTimerManager().ClearTimer(AsyncCheckTimerHandle);
		}
		
		// 완료 델리게이트 호출
		if (!bIsCancelledAtomic.load() && bIsCompleteAtomic.load())
		{
			FWFC3DAlgorithmResult Result;
			Result.bSuccess = true;
			OnExecutionCompleted.Broadcast(Result);
		}
	}
}

void UWFC3DController::SetCurrentPhase(const FString& PhaseName, float PhaseProgress)
{
	CurrentPhaseName = PhaseName;
	OnPhaseChanged.Broadcast(PhaseName, PhaseProgress);
}

void UWFC3DController::UpdateProgress(int32 CurrentStep, int32 TotalSteps)
{
	CurrentStepAtomic = CurrentStep;
	TotalStepsAtomic = TotalSteps;
	OnExecutionProgress.Broadcast(CurrentStep, TotalSteps);
}

void UWFC3DController::BindAlgorithmDelegates()
{
	if (Algorithm)
	{
		Algorithm->OnAlgorithmProgress.AddDynamic(this, &UWFC3DController::OnAlgorithmProgress);
		Algorithm->OnAlgorithmCompleted.AddDynamic(this, &UWFC3DController::OnAlgorithmCompleted);
		Algorithm->OnAlgorithmCancelled.AddDynamic(this, &UWFC3DController::OnAlgorithmCancelled);
	}
}

void UWFC3DController::BindVisualizationDelegates()
{
	if (Visualizer)
	{
		Visualizer->OnVisualizationProgress.AddDynamic(this, &UWFC3DController::OnVisualizationProgress);
		Visualizer->OnVisualizationCompleted.AddDynamic(this, &UWFC3DController::OnVisualizationCompleted);
		Visualizer->OnVisualizationCancelled.AddDynamic(this, &UWFC3DController::OnVisualizationCancelled);
	}
}

void UWFC3DController::UnbindDelegates()
{
	if (Algorithm)
	{
		Algorithm->OnAlgorithmProgress.RemoveDynamic(this, &UWFC3DController::OnAlgorithmProgress);
		Algorithm->OnAlgorithmCompleted.RemoveDynamic(this, &UWFC3DController::OnAlgorithmCompleted);
		Algorithm->OnAlgorithmCancelled.RemoveDynamic(this, &UWFC3DController::OnAlgorithmCancelled);
	}
	
	if (Visualizer)
	{
		Visualizer->OnVisualizationProgress.RemoveDynamic(this, &UWFC3DController::OnVisualizationProgress);
		Visualizer->OnVisualizationCompleted.RemoveDynamic(this, &UWFC3DController::OnVisualizationCompleted);
		Visualizer->OnVisualizationCancelled.RemoveDynamic(this, &UWFC3DController::OnVisualizationCancelled);
	}
}

void UWFC3DController::OnAlgorithmProgress(int32 CurrentStep, int32 TotalSteps)
{
	UpdateProgress(CurrentStep, TotalSteps);
}

void UWFC3DController::OnAlgorithmCompleted(const FWFC3DAlgorithmResult& Result)
{
	// 알고리즘 완료 처리는 ExecuteAlgorithm에서 담당
	// 로그만 찍어
	UE_LOG(LogTemp, Display, TEXT("Controller Execution Algorithm Completed"));
}

void UWFC3DController::OnAlgorithmCancelled()
{
	// 취소 처리는 CancelExecution에서 담당
}

void UWFC3DController::OnVisualizationProgress(int32 CurrentStep, int32 TotalSteps)
{
	UpdateProgress(CurrentStep, TotalSteps);
}

void UWFC3DController::OnVisualizationCompleted(const FWFC3DVisualizeResult& Result)
{
	// 시각화 완료 처리는 ExecuteVisualization에서 담당
}

void UWFC3DController::OnVisualizationCancelled()
{
	// 취소 처리는 CancelExecution에서 담당
}
