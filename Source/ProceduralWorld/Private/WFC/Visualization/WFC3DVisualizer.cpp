// Copyright Epic Games, Inc. All Rights Reserved.

#include "WFC/Visualization/WFC3DVisualizer.h"
#include "WFC/Data/WFC3DTypes.h"
#include "WFC/Data/WFC3DGrid.h"
#include "WFC/Data/WFC3DCell.h"
#include "Async/Async.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "WFC/Data/WFC3DModelDataAsset.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Materials/MaterialInterface.h"
#include "Components/SceneComponent.h"
#include "WFC/Utility/WFC3DHelperFunctions.h"

void FWFC3DVisualizeAsyncTask::DoWork()
{
	if (Visualizer != nullptr)
	{
		Result = Visualizer->ExecuteInternal(Context);
	}
}

void UWFC3DVisualizer::BeginDestroy()
{
	// 비동기 작업이 진행 중이면 취소하고 정리
	if (AsyncTask.IsValid() && !AsyncTask->IsDone())
	{
		CancelExecution();
		AsyncTask->EnsureCompletion();
	}

	// 스폰된 메시들 정리
	CleanupSpawnedMeshes();

	Super::BeginDestroy();
}

FWFC3DVisualizeResult UWFC3DVisualizer::Execute(const FWFC3DVisualizeContext& Context)
{
	return ExecuteInternal(Context);
}

void UWFC3DVisualizer::ExecuteAsync(const FWFC3DVisualizeContext& Context)
{
	// 이미 실행 중이면 무시
	if (bIsRunning)
	{
		return;
	}

	// 상태 초기화
	ResetExecutionState();

	// 비동기 태스크 생성 및 시작
	AsyncTask = MakeUnique<FAsyncTask<FWFC3DVisualizeAsyncTask>>(this, Context);
	AsyncTask->StartBackgroundTask();

	bIsRunning = true;

	// 주기적으로 완료 상태 확인을 위한 타이머 설정
	if (GetWorld())
	{
		GetWorld()->GetTimerManager().SetTimer(
			AsyncCheckTimerHandle,
			FTimerDelegate::CreateLambda([this, Context]()
			{
				CheckAsyncTaskCompletion(Context);
			}),
			0.05f, // 0.05초마다 체크
			true
		);
	}
}

FWFC3DVisualizeResult UWFC3DVisualizer::ExecuteInternal(const FWFC3DVisualizeContext& Context)
{
	FScopeLock Lock(&CriticalSection);

	FWFC3DVisualizeResult Result;

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
		bIsRunning = false;
		bIsRunningAtomic = false;
		return Result;
	}

	if (ModelData == nullptr)
	{
		bIsRunning = false;
		bIsRunningAtomic = false;
		return Result;
	}

	// 전체 단계 수 계산
	TotalSteps = Grid->Num(); // 시각화할 총 셀 수
	CurrentStep = 0;
	TotalStepsAtomic = TotalSteps;
	CurrentStepAtomic = 0;

	// 기존 메시들 정리
	CleanupSpawnedMeshes();


	// 각 셀에 대한 시각화 데이터 준비
	if (TArray<FWFC3DCell>* AllCells = Grid->GetAllCells())
	{
		for (int32 i = 0; i < AllCells->Num() && bIsRunningAtomic.load() && !bIsCancelledAtomic.load(); ++i)
		{
			// 취소 요청이 있으면 루프 중단
			if (bIsCancelledAtomic.load())
			{
				Result.bSuccess = false;
				bIsRunning = false;
				bIsRunningAtomic = false;

				// 메인 스레드에서 취소 델리게이트 호출
				if (GEngine && GEngine->GetWorld())
				{
					GEngine->GetWorld()->GetGameInstance()->GetTimerManager().SetTimerForNextTick([this]()
					{
						OnVisualizationCancelled.Broadcast();
					});
				}

				return Result;
			}

			FWFC3DCell& Cell = (*AllCells)[i];

			// 붕괴된 셀만 시각화 데이터 준비
			if (Cell.bIsCollapsed && Cell.CollapsedTileInfo)
			{
				if (!SetTileVisualInfo(Cell, ModelData, Context.RandomStream))
				{
					// 데이터 준비 실패 시 경고 로그만 출력하고 계속 진행
					UE_LOG(LogTemp, Warning, TEXT("Failed to prepare visualization data for cell at location: %s"), *Cell.Location.ToString());
				}
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
					OnVisualizationProgress.Broadcast(CurrentStepValue, TotalStepsValue);
				});
			}

			// 취소 요청이 다시 확인 (긴 연산 후)
			if (bIsCancelledAtomic.load())
			{
				Result.bSuccess = false;
				bIsRunning = false;
				bIsRunningAtomic = false;

				// 메인 스레드에서 취소 델리게이트 호출
				if (GEngine && GEngine->GetWorld())
				{
					GEngine->GetWorld()->GetGameInstance()->GetTimerManager().SetTimerForNextTick([this]()
					{
						OnVisualizationCancelled.Broadcast();
					});
				}

				return Result;
			}
		}
	}

	// 메시 생성은 Actor에서 담당하므로 여기서는 시각화 데이터만 준비

	// 정상 완료
	Result.bSuccess = true;
	bIsComplete = true;
	bIsRunning = false;
	bIsCompleteAtomic = true;
	bIsRunningAtomic = false;

	return Result;
}

void UWFC3DVisualizer::CancelExecution()
{
	if (bIsRunningAtomic.load())
	{
		bIsCancelled = true;
		bIsCancelledAtomic = true;
	}
}

void UWFC3DVisualizer::ResetExecutionState()
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

	// 스폰된 메시들 정리
	CleanupSpawnedMeshes();

}

float UWFC3DVisualizer::GetProgress() const
{
	int32 Total = TotalStepsAtomic.load();
	if (Total <= 0)
	{
		return 0.0f;
	}

	int32 Current = CurrentStepAtomic.load();
	return FMath::Clamp(static_cast<float>(Current) / static_cast<float>(Total), 0.0f, 1.0f);
}

void UWFC3DVisualizer::CheckAsyncTaskCompletion(const FWFC3DVisualizeContext& Context)
{
	if (!AsyncTask.IsValid())
	{
		return;
	}

	if (AsyncTask->IsDone())
	{
		// 비동기 작업 완료
		FWFC3DVisualizeResult Result = AsyncTask->GetTask().GetResult();

		// 상태 업데이트
		bIsRunning = false;
		bIsComplete = !bIsCancelled && Result.bSuccess;

		// 타이머 정리
		if (GetWorld() && AsyncCheckTimerHandle.IsValid())
		{
			GetWorld()->GetTimerManager().ClearTimer(AsyncCheckTimerHandle);
		}

		// 메시 생성은 Actor에서 담당하므로 여기서는 완료만 처리

		// 완료 델리게이트 호출
		if (!bIsCancelled)
		{
			OnVisualizationCompleted.Broadcast(Result);
		}

		// 비동기 태스크 정리
		AsyncTask.Reset();
	}
}

bool UWFC3DVisualizer::SetTileVisualInfo(FWFC3DCell& Cell, const UWFC3DModelDataAsset* ModelData, const FRandomStream* RandomStream)
{
	if (!Cell.bIsCollapsed || !Cell.CollapsedTileInfo || !ModelData)
	{
		return false;
	}

	// 타일 정보 가져오기
	const FTileInfo* TileInfo = Cell.CollapsedTileInfo;
	int32 BaseTileID = TileInfo->BaseTileID;
	
	// 타일 변형 정보 가져오기
	const FTileVariantInfo* VariantInfo = ModelData->GetTileVariant(BaseTileID);
	if (!VariantInfo)
	{
		return false;
	}

	// TODO: Default 대신에 환경에서 받아오도록 하기(나아아아중에)
	const FTileByBiome& BiomeInfo = VariantInfo->Biomes[DefaultBiomeName];

	TArray<float> TileWeights;
	for (const auto Tile : BiomeInfo.Tiles)
	{
		TileWeights.Add(Tile.Weight);
	}
	
	int32 VariantIndex = FWFC3DHelperFunctions::GetWeightedRandomIndex(TileWeights, RandomStream);

	// 시각 정보 가져오기 (기본 바이옴과 설정된 변형 인덱스 사용)
	const FTileVisualInfo* VisualInfo = ModelData->GetTileVisualInfo(BaseTileID, DefaultBiomeName, VariantIndex);

	// 설정된 인덱스가 범위를 벗어나면 첫 번째 변형으로 fallback
	if (!VisualInfo && DefaultVariantIndex != 0)
	{
		VisualInfo = ModelData->GetTileVisualInfo(BaseTileID, DefaultBiomeName, 0);
		UE_LOG(LogTemp, Warning, TEXT("DefaultVariantIndex %d out of range for BaseTileID %d, falling back to index 0"), DefaultVariantIndex, BaseTileID);
	}

	if (!VisualInfo || !VisualInfo->StaticMesh)
	{
		return false;
	}

	// Cell의 CollapsedTileVisualInfo에 데이터 저장
	Cell.CollapsedTileVisualInfo = const_cast<FTileVisualInfo*>(VisualInfo);

	return true;
}

void UWFC3DVisualizer::CreateMeshesFromData(UWorld* World, UWFC3DGrid* Grid, const UWFC3DModelDataAsset* ModelData)
{
	if (!World || !Grid)
	{
		return;
	}

	// 루트 컴포넌트 생성
	if (!RootComponent)
	{
		RootComponent = NewObject<USceneComponent>(this);
		RootComponent->SetWorldLocation(FVector::ZeroVector);
	}

	if (!RootComponent)
	{
		return;
	}

	// Grid의 모든 셀을 순회하며 메시 생성
	if (TArray<FWFC3DCell>* AllCells = Grid->GetAllCells())
	{
		for (const FWFC3DCell& Cell : *AllCells)
		{
			// 붕괴된 셀이고 시각 정보가 있는 경우만 처리
			if (!Cell.bIsCollapsed || !Cell.CollapsedTileVisualInfo || !Cell.CollapsedTileVisualInfo->StaticMesh)
			{
				continue;
			}

			// 스태틱 메시 컴포넌트 생성
			UStaticMeshComponent* MeshComponent = NewObject<UStaticMeshComponent>(this);
			if (!MeshComponent)
			{
				continue;
			}

			// 메시 설정
			MeshComponent->SetStaticMesh(Cell.CollapsedTileVisualInfo->StaticMesh);

			// 머티리얼 적용
			for (int32 MaterialIndex = 0; MaterialIndex < Cell.CollapsedTileVisualInfo->Materials.Num(); ++MaterialIndex)
			{
				if (Cell.CollapsedTileVisualInfo->Materials[MaterialIndex])
				{
					MeshComponent->SetMaterial(MaterialIndex, Cell.CollapsedTileVisualInfo->Materials[MaterialIndex]);
				}
			}

			// 위치 계산
			FVector Location = FVector(
				Cell.Location.X * TileSize,
				Cell.Location.Y * TileSize,
				Cell.Location.Z * TileSize
			);

			// 회전 계산
			FRotator Rotation = FRotator::ZeroRotator;
			if (ModelData)
			{
				const FTileRotationInfo* RotationInfo = ModelData->GetTileRotationInfo(Cell.CollapsedTileInfoIndex);
				int32 RotationStep = RotationInfo ? RotationInfo->RotationStep : 0;
				Rotation = GetRotationFromRotationStep(RotationStep);
			}

			// 트랜스폼 설정
			FTransform Transform(Rotation, Location, FVector::OneVector);
			MeshComponent->SetWorldTransform(Transform);

			// 루트 컴포넌트에 연결
			MeshComponent->AttachToComponent(RootComponent,
			                                 FAttachmentTransformRules::KeepWorldTransform);

			// 컴포넌트 등록
			MeshComponent->RegisterComponent();

			// 생성된 컴포넌트를 배열에 추가하여 나중에 정리할 수 있도록 함
			SpawnedMeshComponents.Add(MeshComponent);
		}
	}
}

FRotator UWFC3DVisualizer::GetRotationFromRotationStep(int32 RotationStep)
{
	// 회전 스텝을 FRotator로 변환 (Y축 기준 90도씩 회전)
	float YawRotation = RotationStep * 90.0f;
	return FRotator(0.0f, YawRotation, 0.0f);
}

void UWFC3DVisualizer::CleanupSpawnedMeshes()
{
	// 스폰된 메시 컴포넌트들 정리
	for (UStaticMeshComponent* MeshComponent : SpawnedMeshComponents)
	{
		if (IsValid(MeshComponent))
		{
			MeshComponent->DestroyComponent();
		}
	}
	SpawnedMeshComponents.Empty();

	// 루트 컴포넌트 정리
	if (IsValid(RootComponent))
	{
		RootComponent->DestroyComponent();
		RootComponent = nullptr;
	}
}
