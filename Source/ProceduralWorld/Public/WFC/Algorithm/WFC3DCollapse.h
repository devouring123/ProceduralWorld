// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "WFC3DAlgorithmTypes.h"
#include "UObject/Object.h"
#include "WFC3DCollapse.generated.h"

struct FWFC3DCell;
struct FRandomStream;
struct FTileInfo;

class UWFC3DGrid;
class UWFC3DModelDataAsset;

/**
 * WFC3D 알고리즘의 Collapse 함수 모음
 */
namespace
PROCEDURALWORLD_API WFC3DCollapseFunctions
{
	/**
	 * 셀 선택 관련 함수 모음
	 */
	namespace CellSelector
	{
		/**
		 * 엔트로피 기반 Cell 선택 함수
		 * @param Grid - WFC3D 그리드
		 * @param RandomStream - 랜덤 스트림
		 * @return int32 - 선택된 셀 인덱스
		 */
		static int32 ByEntropy(UWFC3DGrid* Grid, const FRandomStream& RandomStream);

		/**
		 * 완전 랜덤 Cell 선택 함수
		 * @param Grid - WFC3D 그리드
		 * @param RandomStream - 랜덤 스트림
		 * @return int32 - 선택된 셀 인덱스
		 */
		static int32 Random(UWFC3DGrid* Grid, const FRandomStream& RandomStream);

		/**
		 * Custom Cell 선택 함수
		 * @param Grid - WFC3D 그리드
		 * @param RandomStream - 랜덤 스트림
		 * @return int32 - 선택된 셀 인덱스
		 */
		static int32 Custom(UWFC3DGrid* Grid, const FRandomStream& RandomStream);
	}

	/**
	 * 타일 정보 선택 관련 함수 모음
	 */
	namespace TileInfoSelector
	{
		/**
		 * 가중치 기반 TileInfo 선택 함수
		 * @param Grid - WFC3D 그리드
		 * @param ModelData - WFC3D 모델 데이터
		 * @param RandomStream - 랜덤 스트림
		 * @param SelectedCellIndex - 선택된 셀 인덱스
		 * @return const FTileInfo* - 선택된 TileInfo
		 */
		static const FTileInfo* ByWeight(UWFC3DGrid* Grid, const UWFC3DModelDataAsset* ModelData, const FRandomStream& RandomStream, int32 SelectedCellIndex);

		/**
		 * 랜덤 TileInfo 선택 함수
		 * @param Grid - WFC3D 그리드
		 * @param ModelData - WFC3D 모델 데이터
		 * @param RandomStream - 랜덤 스트림
		 * @param SelectedCellIndex - 선택된 셀 인덱스
		 * @return const FTileInfo* - 선택된 TileInfo
		 */
		static const FTileInfo* Random(UWFC3DGrid* Grid, const UWFC3DModelDataAsset* ModelData, const FRandomStream& RandomStream, int32 SelectedCellIndex);

		/**
		 * Custom TileInfo 선택 함수
		 * @param Grid - WFC3D 그리드
		 * @param ModelData - WFC3D 모델 데이터
		 * @param RandomStream - 랜덤 스트림
		 * @param SelectedCellIndex - 선택된 셀 인덱스
		 * @return const FTileInfo* - 선택된 TileInfo
		 */
		static const FTileInfo* Custom(UWFC3DGrid* Grid, const UWFC3DModelDataAsset* ModelData, const FRandomStream& RandomStream, int32 SelectedCellIndex);
	}

	/**
	 * 셀 붕괴 관련 함수 모음
	 */
	namespace CellCollapser
	{
		/**
		 * 기본 Cell 붕괴 함수
		 * @param SelectedCell - 붕괴할 Cell
		 * @param SelectedCellIndex - 붕괴할 Cell의 인덱스
		 * @param SelectedTileInfo - 붕괴할 Cell에 들어갈 TileInfo
		 * @return bool - 붕괴 성공 여부
		 */
		static bool Default(FWFC3DCell* SelectedCell, int32 SelectedCellIndex, const FTileInfo* SelectedTileInfo);

		/**
		 * Custom Cell 붕괴 함수
		 * @param SelectedCell - 붕괴할 Cell
		 * @param SelectedCellIndex - 붕괴할 Cell의 인덱스
		 * @param SelectedTileInfo - 붕괴할 Cell에 들어갈 TileInfo
		 * @return bool - 붕괴 성공 여부
		 */
		static bool Custom(FWFC3DCell* SelectedCell, int32 SelectedCellIndex, const FTileInfo* SelectedTileInfo);
	}
}

/**
 * Collapse 전략 구조체 - 각 단계별 함수 포인터를 조합하여 전략 정의
 */
USTRUCT(BlueprintType)
struct PROCEDURALWORLD_API FCollapseStrategy
{
	GENERATED_BODY()

public:
	/** 셀 선택 전략 Enum */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WFC3D")
	ECollapseCellSelectStrategy CellSelectStrategy;

	/** TileInfo 선택 전략 Enum */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WFC3D")
	ECollapseTileSelectStrategy TileSelectStrategy;

	/** 단일 셀 붕괴 전략 Enum */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WFC3D")
	ECollapseCellCollapseStrategy CellCollapseStrategy;

	/** 전략 이름 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WFC3D")
	FString StrategyName;

	
	FCollapseStrategy()
		: CellSelectStrategy(ECollapseCellSelectStrategy::ByEntropy),
		  TileSelectStrategy(ECollapseTileSelectStrategy::ByWeight),
		  CellCollapseStrategy(ECollapseCellCollapseStrategy::Default),
		  StrategyName(TEXT("Standard"))
	{
	}
	
	FCollapseStrategy(
		ECollapseCellSelectStrategy InCellSelectStrategy,
		ECollapseTileSelectStrategy InTileSelectStrategy,
		ECollapseCellCollapseStrategy InCellCollapseStrategy,
		const FString& InStrategyName)
		: CellSelectStrategy(InCellSelectStrategy),
		  TileSelectStrategy(InTileSelectStrategy),
		  CellCollapseStrategy(InCellCollapseStrategy),
		  StrategyName(InStrategyName)
	{
	}


	/** TODO: Delete This Function */
	/**
	 * 이 전략에 기반한 Collapse 실행
	 * @param Grid - WFC3D 그리드
	 * @param ModelData - WFC3D 모델 데이터
	 * @param RandomStream - 랜덤 스트림
	 * @return FCollapseResult - 붕괴 결과
	 */
	FCollapseResult ExecuteCollapse(UWFC3DGrid* Grid, const UWFC3DModelDataAsset* ModelData, const FRandomStream& RandomStream) const;
};

/** TODO: Add Static Maps (Enum -> FuncPtr) */
/** TODO: Macro를 사용하여 Enum과 FuncPtr를 연결, Enum이름과 FuncPtr의 이름을 동일하게 작성 */
/**
 * WFC3D 알고리즘의 Collapse 전략 관리
 */
UCLASS(Blueprintable)
class PROCEDURALWORLD_API UWFC3DCollapseStrategyManager : public UObject
{
	GENERATED_BODY()

public:
	/** 기본 전략 생성 - 엔트로피 기반 셀 선택, 가중치 기반 타일 선택, 기본 붕괴 */
	static FCollapseStrategy CreateStandardStrategy();

	/** 가중치 전략 생성 - 랜덤 셀 선택, 가중치 기반 타일 선택, 기본 붕괴 */
	static FCollapseStrategy CreateWeightedStrategy();

	/** 랜덤 전략 생성 - 랜덤 셀 선택, 랜덤 타일 선택, 기본 붕괴 */
	static FCollapseStrategy CreateRandomStrategy();

	/** 사용자 정의 전략 생성 */
	static FCollapseStrategy CreateCustomStrategy(
		SelectCellFunc CellSelectorFunc,
		SelectTileInfoFunc TileInfoSelectorFunc,
		CollapseSingleCellFunc CellCollapserFunc,
		const FString& StrategyName = TEXT("Custom"));
};
