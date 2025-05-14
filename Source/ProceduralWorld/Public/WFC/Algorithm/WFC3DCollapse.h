// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "WFC3DAlgorithmMacros.h"
#include "WFC3DAlgorithmTypes.h"
#include "UObject/Object.h"
#include "WFC3DCollapse.generated.h"

struct FWFC3DCell;
struct FRandomStream;
struct FTileInfo;

class UWFC3DGrid;
class UWFC3DModelDataAsset;

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
	ECollapseTileInfoSelectStrategy TileSelectStrategy;

	/** 단일 셀 붕괴 전략 Enum */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WFC3D")
	ECollapseSingleCellStrategy CellCollapseStrategy;
	
	FCollapseStrategy()
		: CellSelectStrategy(ECollapseCellSelectStrategy::ByEntropy),
		  TileSelectStrategy(ECollapseTileInfoSelectStrategy::ByWeight),
		  CellCollapseStrategy(ECollapseSingleCellStrategy::Default)
	{
	}

	FCollapseStrategy(
		ECollapseCellSelectStrategy InCellSelectStrategy,
		ECollapseTileInfoSelectStrategy InTileSelectStrategy,
		ECollapseSingleCellStrategy InCellCollapseStrategy)
		: CellSelectStrategy(InCellSelectStrategy),
		  TileSelectStrategy(InTileSelectStrategy),
		  CellCollapseStrategy(InCellCollapseStrategy)
	{
	}
};

/**
 * WFC3D 알고리즘의 Collapse 함수 모음
 */
namespace
PROCEDURALWORLD_API WFC3DCollapseFunctions
{
	/** 전략에 맞는 붕괴 함수 실행 */
	FCollapseResult ExecuteCollapse(UWFC3DGrid* Grid, const UWFC3DModelDataAsset* ModelData, const FRandomStream& RandomStream, const FCollapseStrategy& CollapseStrategy);

	/**
	 * 셀 선택 관련 함수 모음
	 */
	namespace CellSelector
	{
		/**
		 * 엔트로피 기반 Cell 선택 함수
		 */
		DECLARE_COLLAPSER_CELL_SELECTOR_STRATEGY(ByEntropy);

		/**
		 * 완전 랜덤 Cell 선택 함수
		 */
		DECLARE_COLLAPSER_CELL_SELECTOR_STRATEGY(Random);

		/**
		 * 붕괴한 셀 인접 Cell 선택 함수
		 */
		// DECLARE_COLLAPSER_CELL_SELECTOR_STRATEGY(Adjacent);
		
		/**
		 * Custom Cell 선택 함수
		 */
		DECLARE_COLLAPSER_CELL_SELECTOR_STRATEGY(Custom);
	}

	/**
	 * 타일 정보 선택 관련 함수 모음
	 */
	namespace TileInfoSelector
	{
		/**
		 * 가중치 기반 TileInfo 선택 함수
		 */
		DECLARE_COLLAPSER_TILE_SELECTOR_STRATEGY(ByWeight);

		/**
		 * 랜덤 TileInfo 선택 함수
		 */
		DECLARE_COLLAPSER_TILE_SELECTOR_STRATEGY(Random);

		/**
		 * Custom TileInfo 선택 함수
		 */
		DECLARE_COLLAPSER_TILE_SELECTOR_STRATEGY(Custom);
	}

	/**
	 * 셀 붕괴 관련 함수 모음
	 */
	namespace CellCollapser
	{
		/**
		 * 기본 Cell 붕괴 함수
		 */
		DECLARE_COLLAPSER_CELL_COLLAPSER_STRATEGY(Default);

		/**
		 * Custom Cell 붕괴 함수
		 */
		DECLARE_COLLAPSER_CELL_COLLAPSER_STRATEGY(Custom);
	}
}

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
