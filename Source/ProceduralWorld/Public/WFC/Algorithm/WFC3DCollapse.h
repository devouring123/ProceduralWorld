// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "WFC/Data/WFC3DTypes.h"
#include "WFC3DAlgorithmMacros.h"
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

	/** TileInfo Index 선택 전략 Enum */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WFC3D")
	ECollapseTileInfoIndexSelectStrategy TileInfoIndexSelectStrategy;

	/** 단일 셀 붕괴 전략 Enum */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WFC3D")
	ECollapseSingleCellStrategy CellCollapseStrategy;
	
	FCollapseStrategy()
		: CellSelectStrategy(ECollapseCellSelectStrategy::ByEntropy),
		  TileInfoIndexSelectStrategy(ECollapseTileInfoIndexSelectStrategy::ByWeight),
		  CellCollapseStrategy(ECollapseSingleCellStrategy::Default)
	{
	}

	FCollapseStrategy(
		ECollapseCellSelectStrategy InCellSelectStrategy,
		ECollapseTileInfoIndexSelectStrategy InTileSelectStrategy,
		ECollapseSingleCellStrategy InCellCollapseStrategy)
		: CellSelectStrategy(InCellSelectStrategy),
		  TileInfoIndexSelectStrategy(InTileSelectStrategy),
		  CellCollapseStrategy(InCellCollapseStrategy)
	{
	}
};

/**
 * WFC3D 알고리즘의 Collapse 함수 모음
 */
namespace WFC3DCollapseFunctions
{
	/**
	 * 셀 붕괴
	 * @param Context - WFC3D Collapse Context
	 * @param SelectCellFuncPtr - 셀 선택 함수 포인터
	 * @param SelectTileInfoIndexFuncPtr - 타일 정보 선택 함수 포인터
	 * @param CollapseSingleCellFuncPtr - 단일 셀 붕괴 함수 포인터
	 * @return FCollapseResult - 붕괴 결과
	 */
	FCollapseResult ExecuteCollapse(const FWFC3DCollapseContext& Context, SelectCellFunc SelectCellFuncPtr, SelectTileInfoIndexFunc SelectTileInfoIndexFuncPtr,
	                                CollapseSingleCellFunc CollapseSingleCellFuncPtr);
	
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