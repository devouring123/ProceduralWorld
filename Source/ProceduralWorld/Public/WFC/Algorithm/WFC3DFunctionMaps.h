// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "WFC/Algorithm/WFC3DAlgorithmTypes.h"

/**
 * WFC3D 알고리즘의 전략 함수 매핑 관리
 */
class PROCEDURALWORLD_API FWFC3DFunctionMaps
{
public:
	/** 셀 선택 함수 포인터 획득 */
	static SelectCellFunc GetCellSelectorFunction(ECollapseCellSelectStrategy Strategy);

	/** 타일 정보 선택 함수 포인터 획득 */
	static SelectTileInfoIndexFunc GetTileInfoIndexSelectorFunction(ECollapseTileInfoIndexSelectStrategy Strategy);

	/** 셀 붕괴 함수 포인터 획득 */
	static CollapseSingleCellFunc GetCellCollapserFunction(ECollapseSingleCellStrategy Strategy);

	/** 전파 반경 제한 함수 포인터 획득 */
	static RangeLimitFunc GetRangeLimitFunction(ERangeLimitStrategy Strategy);
	
	/** 전략 등록 함수들 */
	static void RegisterCellSelectorEnum(ECollapseCellSelectStrategy Enum, SelectCellFunc Function);
	static void RegisterTileInfoIndexSelectorEnum(ECollapseTileInfoIndexSelectStrategy Enum, SelectTileInfoIndexFunc Function);
	static void RegisterCellCollapserEnum(ECollapseSingleCellStrategy Enum, CollapseSingleCellFunc Function);
	static void RegisterRangeLimitEnum(ERangeLimitStrategy Enum, RangeLimitFunc Function);
	
private:
	/** 유틸리티 클래스 생성자 및 소멸자 제거 */
	FWFC3DFunctionMaps() = delete;
	FWFC3DFunctionMaps(const FWFC3DFunctionMaps&) = delete;
	FWFC3DFunctionMaps& operator=(const FWFC3DFunctionMaps&) = delete;
	FWFC3DFunctionMaps(FWFC3DFunctionMaps&&) = delete;
	FWFC3DFunctionMaps& operator=(FWFC3DFunctionMaps&&) = delete;
	~FWFC3DFunctionMaps() = delete;

	/** 셀 선택 전략 맵 */
	static TMap<ECollapseCellSelectStrategy, SelectCellFunc> CellSelectorMap;

	/** 타일 정보 선택 전략 맵 */
	static TMap<ECollapseTileInfoIndexSelectStrategy, SelectTileInfoIndexFunc> TileInfoIndexSelectorMap;

	/** 셀 붕괴 전략 맵 */
	static TMap<ECollapseSingleCellStrategy, CollapseSingleCellFunc> CellCollapserMap;

	/** 전파 반경 제한 전략 맵 */
	static TMap<ERangeLimitStrategy, RangeLimitFunc> RangeLimitMap;
};
