// Copyright Epic Games, Inc. All Rights Reserved.

#include "CoreMinimal.h"
#include "WFC/Algorithm/WFC3DAlgorithm.h"
#include "WFC/Data/WFC3DGrid.h"

/**
 * WFC3D 알고리즘 간단한 테스트
 */
void TestWFC3DAlgorithm()
{
	UE_LOG(LogTemp, Warning, TEXT("=== WFC3D 알고리즘 테스트 시작 ==="));
	
	// 1. Grid 생성 및 초기화
	UWFC3DGrid* TestGrid = NewObject<UWFC3DGrid>();
	if (!TestGrid)
	{
		UE_LOG(LogTemp, Error, TEXT("❌ Grid 생성 실패"));
		return;
	}
	
	// Grid 상태 확인
	UE_LOG(LogTemp, Log, TEXT("Grid 생성 완료:"));
	UE_LOG(LogTemp, Log, TEXT("  - Dimension: %s"), *TestGrid->GetDimension().ToString());
	UE_LOG(LogTemp, Log, TEXT("  - Total Cells: %d"), TestGrid->Num());
	UE_LOG(LogTemp, Log, TEXT("  - Remaining Cells: %d"), TestGrid->GetRemainingCells());
	
	// 2. 알고리즘 인스턴스 생성
	UWFC3DAlgorithm* Algorithm = NewObject<UWFC3DAlgorithm>();
	if (!Algorithm)
	{
		UE_LOG(LogTemp, Error, TEXT("❌ 알고리즘 인스턴스 생성 실패"));
		return;
	}
	
	UE_LOG(LogTemp, Log, TEXT("✅ 알고리즘 인스턴스 생성 성공"));
	
	// 3. Context 생성 (ModelData는 nullptr로 설정하여 테스트)
	FWFC3DAlgorithmContext TestContext(TestGrid, nullptr);
	
	// 4. 초기 상태 확인
	UE_LOG(LogTemp, Log, TEXT("초기 상태:"));
	UE_LOG(LogTemp, Log, TEXT("  - 실행 중: %s"), Algorithm->IsRunning() ? TEXT("예") : TEXT("아니오"));
	UE_LOG(LogTemp, Log, TEXT("  - 취소됨: %s"), Algorithm->IsCancelled() ? TEXT("예") : TEXT("아니오"));
	UE_LOG(LogTemp, Log, TEXT("  - 완료됨: %s"), Algorithm->IsComplete() ? TEXT("예") : TEXT("아니오"));
	UE_LOG(LogTemp, Log, TEXT("  - 진행률: %.1f%%"), Algorithm->GetProgress() * 100.0f);
	
	// 5. 동기 실행 테스트 (ModelData가 nullptr이므로 에러로 종료될 것)
	UE_LOG(LogTemp, Log, TEXT("동기 실행 테스트 시작..."));
	FWFC3DResult Result = Algorithm->Execute(TestContext);
	
	UE_LOG(LogTemp, Log, TEXT("동기 실행 결과:"));
	UE_LOG(LogTemp, Log, TEXT("  - 성공: %s"), Result.bSuccess ? TEXT("예") : TEXT("아니오"));
	UE_LOG(LogTemp, Log, TEXT("  - Collapse 결과 수: %d"), Result.CollapseResults.Num());
	UE_LOG(LogTemp, Log, TEXT("  - Propagation 결과 수: %d"), Result.PropagationResults.Num());
	
	// 6. 상태 리셋 테스트
	Algorithm->ResetExecutionState();
	UE_LOG(LogTemp, Log, TEXT("✅ 상태 리셋 완료"));
	
	// 7. 취소 기능 테스트
	Algorithm->CancelExecution();
	UE_LOG(LogTemp, Log, TEXT("✅ 취소 기능 테스트 완료"));
	
	UE_LOG(LogTemp, Warning, TEXT("=== WFC3D 알고리즘 테스트 완료 ==="));
}

/**
 * Grid만 테스트하는 함수
 */
void TestWFC3DGrid()
{
	UE_LOG(LogTemp, Warning, TEXT("=== WFC3D Grid 테스트 시작 ==="));
	
	// 1. 기본 생성자 테스트
	UWFC3DGrid* DefaultGrid = NewObject<UWFC3DGrid>();
	if (DefaultGrid)
	{
		UE_LOG(LogTemp, Log, TEXT("기본 Grid:"));
		UE_LOG(LogTemp, Log, TEXT("  - Dimension: %s"), *DefaultGrid->GetDimension().ToString());
		UE_LOG(LogTemp, Log, TEXT("  - Total Cells: %d"), DefaultGrid->Num());
		UE_LOG(LogTemp, Log, TEXT("  - Remaining Cells: %d"), DefaultGrid->GetRemainingCells());
	}
	
	// 2. 커스텀 크기 Grid 테스트
	FIntVector CustomDimension(3, 3, 3);
	UWFC3DGrid* CustomGrid = NewObject<UWFC3DGrid>();
	// 참고: UObject의 경우 생성자에 파라미터를 전달하기 어려우므로 별도 초기화 함수가 필요할 수 있음
	
	UE_LOG(LogTemp, Warning, TEXT("=== WFC3D Grid 테스트 완료 ==="));
}
