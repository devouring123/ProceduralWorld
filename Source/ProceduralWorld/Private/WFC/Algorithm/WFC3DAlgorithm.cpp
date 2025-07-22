// Copyright Epic Games, Inc. All Rights Reserved.

#include "WFC/Algorithm/WFC3DAlgorithm.h"
#include "WFC/Algorithm/WFC3DAlgorithmTypes.h"
#include "WFC/Algorithm/WFC3DCollapse.h"
#include "WFC/Algorithm/WFC3DPropagation.h"
#include "WFC/Algorithm/WFC3DFunctionMaps.h"
#include "WFC/Data/WFC3DGrid.h"
#include "Async/Async.h"
#include "Engine/Engine.h"
#include "WFC/Data/WFC3DModelDataAsset.h"

void FWFC3DAsyncTask::DoWork()
{
	if (Algorithm != nullptr)
	{
		UE_LOG(LogTemp, Display, TEXT("Async Task started for WFC3D Algorithm DO WORK"));
		UE_LOG(LogTemp, Display, TEXT("Context : Grid: %s"), *Context.Grid->GetDimension().ToString());
		UE_LOG(LogTemp, Display, TEXT("Context : ModelData: %s"), Context.ModelData ? TEXT("Valid") : TEXT("Invalid"));
		if (Context.ModelData)
		{
			UE_LOG(LogTemp, Display, TEXT("Context : ModelData Name: %s"), *Context.ModelData->GetName());
		}
		Result = Algorithm->ExecuteInternal(Context);
	}
}

void UWFC3DAlgorithm::BeginDestroy()
{
	// 비동기 작업이 진행 중이면 취소하고 정리
	if (AsyncTask.IsValid() && !AsyncTask->IsDone())
	{
		CancelExecution();
		AsyncTask->EnsureCompletion();
	}

	Super::BeginDestroy();
}

FWFC3DResult UWFC3DAlgorithm::Execute(const FWFC3DAlgorithmContext& Context)
{
	return ExecuteInternal(Context);
}

void UWFC3DAlgorithm::ExecuteAsync(const FWFC3DAlgorithmContext& Context)
{
	// 이미 실행 중이면 무시
	if (bIsRunning)
	{
		UE_LOG(LogTemp, Warning, TEXT("WFC3D Algorithm is already running"));
		return;
	}

	// 상태 초기화
	ResetExecutionState();

	// 비동기 태스크 생성 및 시작
	AsyncTask = MakeUnique<FAsyncTask<FWFC3DAsyncTask>>(this, Context);
	AsyncTask->StartBackgroundTask();

	bIsRunning = true;

	UE_LOG(LogTemp, Log, TEXT("WFC3D Algorithm started asynchronously"));

	// 주기적으로 완료 상태 확인을 위한 타이머 설정
	if (GetWorld())
	{
		GetWorld()->GetTimerManager().SetTimer(
			AsyncCheckTimerHandle,
			FTimerDelegate::CreateUObject(this, &UWFC3DAlgorithm::CheckAsyncTaskCompletion),
			0.1f, // 0.1초마다 체크
			true
		);
	}
}

FWFC3DResult UWFC3DAlgorithm::ExecuteInternal(const FWFC3DAlgorithmContext& Context)
{
	FScopeLock Lock(&CriticalSection);

	FWFC3DResult Result;

	// 실행 상태 초기화
	bIsRunning = true;
	bIsCancelled = false;
	bIsComplete = false;

	// Atomic 변수들도 초기화
	bIsRunningAtomic = true;
	bIsCancelledAtomic = false;
	bIsCompleteAtomic = false;

	UWFC3DGrid* Grid = Context.Grid;
	const UWFC3DModelDataAsset* ModelData = Context.ModelData;

	if (Grid == nullptr)
	{
		UE_LOG(LogTemp, Error, TEXT("Invalid Grid in Algorithm Context"));
		bIsRunning = false;
		bIsRunningAtomic = false;
		return Result;
	}

	if (ModelData == nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("ModelData is null - using test mode"));
		bIsRunning = false;
		bIsRunningAtomic = false;
		return Result;
	}

	// 전체 단계 수 계산 및 디버깅 로그
	TotalSteps = Grid->GetRemainingCells();
	CurrentStep = 0;
	TotalStepsAtomic = TotalSteps;
	CurrentStepAtomic = 0;

	UE_LOG(LogTemp, Warning, TEXT("=== WFC3D Algorithm Debug Info ==="));
	UE_LOG(LogTemp, Warning, TEXT("Grid Dimension: %s"), *Grid->GetDimension().ToString());
	UE_LOG(LogTemp, Warning, TEXT("Grid Total Cells: %d"), Grid->Num());
	UE_LOG(LogTemp, Warning, TEXT("Grid Remaining Cells: %d"), Grid->GetRemainingCells());
	UE_LOG(LogTemp, Warning, TEXT("Total Steps: %d"), TotalSteps);
	UE_LOG(LogTemp, Warning, TEXT("bIsRunningAtomic: %s"), bIsRunningAtomic.load() ? TEXT("true") : TEXT("false"));
	UE_LOG(LogTemp, Warning, TEXT("bIsCancelledAtomic: %s"), bIsCancelledAtomic.load() ? TEXT("true") : TEXT("false"));
	UE_LOG(LogTemp, Warning, TEXT("While 조건: RemainingCells > 0 && bIsRunning && !bIsCancelled"));
	UE_LOG(LogTemp, Warning, TEXT("While 조건 평가: %s && %s && %s = %s"),
	       Grid->GetRemainingCells() > 0 ? TEXT("true") : TEXT("false"),
	       bIsRunningAtomic.load() ? TEXT("true") : TEXT("false"),
	       !bIsCancelledAtomic.load() ? TEXT("true") : TEXT("false"),
	       (Grid->GetRemainingCells() > 0 && bIsRunningAtomic.load() && !bIsCancelledAtomic.load()) ? TEXT("true") : TEXT("false")
	);
	Grid->PrintGridInfo();
	UE_LOG(LogTemp, Warning, TEXT("==================================="));

	// while 루프 진입 전 확인
	if (Grid->GetRemainingCells() <= 0)
	{
		UE_LOG(LogTemp, Error, TEXT("❌ Grid has no remaining cells to collapse! RemainingCells = %d"), Grid->GetRemainingCells());

		// 전체 셀 상태 확인
		if (TArray<FWFC3DCell>* AllCells = Grid->GetAllCells())
		{
			int32 CollapsedCount = 0;
			int32 UnCollapsedCount = 0;

			for (const FWFC3DCell& Cell : *AllCells)
			{
				if (Cell.bIsCollapsed)
				{
					CollapsedCount++;
				}
				else
				{
					UnCollapsedCount++;
				}
			}

			UE_LOG(LogTemp, Warning, TEXT("📊 Cell Status: Total=%d, Collapsed=%d, UnCollapsed=%d"),
			       AllCells->Num(), CollapsedCount, UnCollapsedCount);
		}

		// 빈 결과 반환
		Result.bSuccess = false;
		bIsRunning = false;
		bIsRunningAtomic = false;
		return Result;
	}

	// Collapse Context 생성
	FWFC3DCollapseContext CollapseContext(Grid, ModelData, &RandomStream);

	// Collapse 함수 포인터 획득
	SelectCellFunc SelectCellFuncPtr = nullptr;
	SelectTileInfoFunc SelectTileInfoFuncPtr = nullptr;
	CollapseSingleCellFunc CollapseSingleCellFuncPtr = nullptr;

	// ModelData가 있을 때만 실제 함수 포인터 획득
	if (ModelData != nullptr)
	{
		SelectCellFuncPtr = FWFC3DFunctionMaps::GetCellSelectorFunction(CollapseStrategy.CellSelectStrategy);
		SelectTileInfoFuncPtr = FWFC3DFunctionMaps::GetTileInfoSelectorFunction(CollapseStrategy.TileSelectStrategy);
		CollapseSingleCellFuncPtr = FWFC3DFunctionMaps::GetCellCollapserFunction(CollapseStrategy.CellCollapseStrategy);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("⚠️ 테스트 모드: ModelData가 null이므로 함수 포인터를 건너뜁니다"));
	}

	if (ModelData != nullptr && (SelectCellFuncPtr == nullptr || SelectTileInfoFuncPtr == nullptr || CollapseSingleCellFuncPtr == nullptr))
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to get Collapse function pointers"));

		UE_LOG(LogTemp, Error, TEXT("ModelData is %s"), ModelData ? TEXT("valid") : TEXT("null"));
		UE_LOG(LogTemp, Error, TEXT("SelectCellFuncPtr is %s"), SelectCellFuncPtr ? TEXT("valid") : TEXT("null"));
		UE_LOG(LogTemp, Error, TEXT("SelectTileInfoFuncPtr is %s"), SelectTileInfoFuncPtr ? TEXT("valid") : TEXT("null"));
		UE_LOG(LogTemp, Error, TEXT("CollapseSingleCellFuncPtr is %s"), CollapseSingleCellFuncPtr ? TEXT("valid") : TEXT("null"));
		
		
		bIsRunning = false;
		bIsRunningAtomic = false;
		return Result;
	}

	while (Grid->GetRemainingCells() > 0 && bIsRunningAtomic.load() && !bIsCancelledAtomic.load())
	{
		UE_LOG(LogTemp, Display, TEXT("🔄 Left Step: %d, Total Steps: %d"), Grid->GetRemainingCells(), TotalStepsAtomic.load());

		// 취소 요청이 있으면 루프 중단
		if (bIsCancelledAtomic.load())
		{
			UE_LOG(LogTemp, Warning, TEXT("WFC3D Algorithm execution was cancelled"));
			Result.bSuccess = false;
			bIsRunning = false;
			bIsRunningAtomic = false;

			// 메인 스레드에서 취소 델리게이트 호출
			if (GEngine && GEngine->GetWorld())
			{
				GEngine->GetWorld()->GetGameInstance()->GetTimerManager().SetTimerForNextTick([this]()
				{
					OnAlgorithmCancelled.Broadcast();
				});
			}

			return Result;
		}

		// 테스트 모드: ModelData가 없으면 간단한 더미 동작 수행
		if (ModelData == nullptr)
		{
			UE_LOG(LogTemp, Log, TEXT("🧪 테스트 모드: 더미 Collapse 및 Propagation 수행"));

			// 더미 Collapse 결과 생성
			FCollapseResult CollapseResult;
			CollapseResult.bSuccess = true;
			CollapseResult.CollapsedIndex = CurrentStep;
			CollapseResult.CollapsedLocation = FIntVector(
				CurrentStep % Grid->GetDimension().X,
				(CurrentStep / Grid->GetDimension().X) % Grid->GetDimension().Y,
				CurrentStep / (Grid->GetDimension().X * Grid->GetDimension().Y)
			);

			Result.CollapseResults.Add(CollapseResult);

			// 더미 Propagation 결과 생성
			FPropagationResult PropagationResult;
			PropagationResult.bSuccess = true;
			PropagationResult.AffectedCellCount = FMath::RandRange(1, 5);

			Result.PropagationResults.Add(PropagationResult);

			// RemainingCells 감소 (테스트 목적)
			Grid->DecreaseRemainingCells();

			UE_LOG(LogTemp, Display, TEXT("🧪 테스트 Collapse at %s, Affected %d cells"),
			       *CollapseResult.CollapsedLocation.ToString(),
			       PropagationResult.AffectedCellCount);
		}
		else
		{
			// 실제 WFC 알고리즘 실행
			FCollapseResult CollapseResult = WFC3DCollapseFunctions::ExecuteCollapse(
				CollapseContext,
				SelectCellFuncPtr,
				SelectTileInfoFuncPtr,
				CollapseSingleCellFuncPtr
			);

			Result.CollapseResults.Add(CollapseResult);

			if (!CollapseResult.bSuccess)
			{
				UE_LOG(LogTemp, Error, TEXT("Collapse failed"));
				bIsRunning = false;
				bIsRunningAtomic = false;
				return Result;
			}

			if (CollapseResult.bSuccess && Grid->GetRemainingCells() == 0)
			{
				// 모든 셀 붕괴 완료
				UE_LOG(LogTemp, Display, TEXT("Collapse Success!! In Algorithm"));
				break;
			}
			
			// Propagation Context 생성
			FWFC3DPropagationContext PropagationContext(Grid, ModelData, CollapseResult.CollapsedLocation);

			// Propagation 실행
			FPropagationResult PropagationResult = WFC3DPropagateFunctions::ExecutePropagation(PropagationContext, PropagationStrategy);

			Result.PropagationResults.Add(PropagationResult);

			if (!PropagationResult.bSuccess)
			{
				UE_LOG(LogTemp, Error, TEXT("Propagation failed In Algorithm.cpp"));
				bIsRunning = false;
				bIsRunningAtomic = false;
				return Result;
			}

			UE_LOG(LogTemp, Display, TEXT("WFC3D Algorithm executed successfully. Collapsed at %s, Affected %d cells"),
			       *CollapseResult.CollapsedLocation.ToString(),
			       PropagationResult.AffectedCellCount);
		}

		// 진행률 업데이트
		CurrentStep++;
		CurrentStepAtomic = CurrentStep;

		// 메인 스레드에서 진행률 델리게이트 호출
		if (GEngine && GEngine->GetWorld())
		{
			int32 CurrentStepValue = CurrentStepAtomic.load();
			int32 TotalStepsValue = TotalStepsAtomic.load();
			GEngine->GetWorld()->GetGameInstance()->GetTimerManager().SetTimerForNextTick([this, CurrentStepValue, TotalStepsValue]()
			{
				OnAlgorithmProgress.Broadcast(CurrentStepValue, TotalStepsValue);
			});
		}

		UE_LOG(LogTemp, Display, TEXT("📊 Progress: %d/%d (%.1f%%)"),
		       CurrentStepAtomic.load(),
		       TotalStepsAtomic.load(),
		       GetProgress() * 100.0f);

		// 취소 요청이 다시 확인 (긴 연산 후)
		if (bIsCancelledAtomic.load())
		{
			UE_LOG(LogTemp, Warning, TEXT("WFC3D Algorithm execution was cancelled during iteration"));
			Result.bSuccess = false;
			bIsRunning = false;
			bIsRunningAtomic = false;

			// 메인 스레드에서 취소 델리게이트 호출
			if (GEngine && GEngine->GetWorld())
			{
				GEngine->GetWorld()->GetGameInstance()->GetTimerManager().SetTimerForNextTick([this]()
				{
					OnAlgorithmCancelled.Broadcast();
				});
			}

			return Result;
		}

		// 테스트 모드에서는 약간의 지연을 추가하여 진행률을 볼 수 있게 함
		if (ModelData == nullptr)
		{
			FPlatformProcess::Sleep(0.1f); // 100ms 대기
		}
	}

	// 정상 완료
	Result.bSuccess = true;
	bIsComplete = true;
	bIsRunning = false;
	bIsCompleteAtomic = true;
	bIsRunningAtomic = false;

	UE_LOG(LogTemp, Log, TEXT("WFC3D Algorithm completed successfully"));

	for (const FWFC3DCell& Cell : *Grid->GetAllCells())
	{
		Cell.PrintTileInfo();
		for (int32 i = 0; i < Cell.CollapsedTileInfo->Faces.Num(); ++i)
		{
			UE_LOG(LogTemp, Display, TEXT("Face %d: %s"), i, *ModelData->GetFaceInfo(Cell.CollapsedTileInfo->Faces[i])->Name);
		}
	}
	
	return Result;
}

void UWFC3DAlgorithm::CancelExecution()
{
	if (bIsRunningAtomic.load())
	{
		bIsCancelled = true;
		bIsCancelledAtomic = true;
		UE_LOG(LogTemp, Warning, TEXT("WFC3D Algorithm cancellation requested"));
	}
}

void UWFC3DAlgorithm::ResetExecutionState()
{
	FScopeLock Lock(&CriticalSection);

	bIsRunning = false;
	bIsCancelled = false;
	bIsComplete = false;
	CurrentStep = 0;
	TotalSteps = 0;

	// Atomic 변수들도 동기화
	bIsRunningAtomic = false;
	bIsCancelledAtomic = false;
	bIsCompleteAtomic = false;
	CurrentStepAtomic = 0;
	TotalStepsAtomic = 0;

	// 타이머 정리
	if (GetWorld() && AsyncCheckTimerHandle.IsValid())
	{
		GetWorld()->GetTimerManager().ClearTimer(AsyncCheckTimerHandle);
	}

	UE_LOG(LogTemp, Log, TEXT("WFC3D Algorithm execution state reset"));
}

float UWFC3DAlgorithm::GetProgress() const
{
	int32 Total = TotalStepsAtomic.load();
	if (Total <= 0)
	{
		return 0.0f;
	}

	int32 Current = CurrentStepAtomic.load();
	return FMath::Clamp(static_cast<float>(Current) / static_cast<float>(Total), 0.0f, 1.0f);
}

void UWFC3DAlgorithm::CheckAsyncTaskCompletion()
{
	if (!AsyncTask.IsValid())
	{
		return;
	}

	if (AsyncTask->IsDone())
	{
		// 비동기 작업 완료
		FWFC3DResult Result = AsyncTask->GetTask().GetResult();

		// 상태 업데이트
		bIsRunning = false;
		bIsComplete = !bIsCancelled && Result.bSuccess;

		// 타이머 정리
		if (GetWorld() && AsyncCheckTimerHandle.IsValid())
		{
			GetWorld()->GetTimerManager().ClearTimer(AsyncCheckTimerHandle);
		}

		// 완료 델리게이트 호출
		if (!bIsCancelled)
		{
			OnAlgorithmCompleted.Broadcast(Result);
		}

		// 비동기 태스크 정리
		AsyncTask.Reset();

		UE_LOG(LogTemp, Log, TEXT("WFC3D Algorithm async task completed"));
	}
}
