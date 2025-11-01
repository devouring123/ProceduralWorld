// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "WFC/Data/WFC3DTypes.h"

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

	/**
	 * 전략 등록 함수들
	 * C++20 Concepts를 사용하여 컴파일 타임에 함수 시그니처 검증
	 *
	 * 언리얼 엔진 코딩 표준:
	 * - 템플릿 매개변수는 T 접두사 사용
	 * - 함수 매개변수는 In/Out 접두사 사용
	 */

	// Cell 선택 전략 등록 (Concept 제약 적용)
	template<TSelectCellFuncConcept TFunc>
	static void RegisterCellSelectorEnum(ECollapseCellSelectStrategy InStrategy, TFunc InFunction)
	{
		// Concept을 만족하는지 컴파일 타임에 검증됨
		CellSelectorMap.Add(InStrategy, FWFC3DFunctionValidator::MakeSelectCellFunction(InFunction));
	}

	// TileInfo 인덱스 선택 전략 등록 (Concept 제약 적용)
	template<TSelectTileInfoIndexFuncConcept TFunc>
	static void RegisterTileInfoIndexSelectorEnum(ECollapseTileInfoIndexSelectStrategy InStrategy, TFunc InFunction)
	{
		// Concept을 만족하는지 컴파일 타임에 검증됨
		TileInfoIndexSelectorMap.Add(InStrategy, FWFC3DFunctionValidator::MakeSelectTileInfoIndexFunction(InFunction));
	}

	// Cell 붕괴 전략 등록 (Concept 제약 적용)
	template<TCollapseSingleCellFuncConcept TFunc>
	static void RegisterCellCollapserEnum(ECollapseSingleCellStrategy InStrategy, TFunc InFunction)
	{
		// Concept을 만족하는지 컴파일 타임에 검증됨
		CellCollapserMap.Add(InStrategy, FWFC3DFunctionValidator::MakeCollapseSingleCellFunction(InFunction));
	}

	// Range Limit 전략 등록 (Concept 제약 적용)
	template<TRangeLimitFuncConcept TFunc>
	static void RegisterRangeLimitEnum(ERangeLimitStrategy InStrategy, TFunc InFunction)
	{
		// Concept을 만족하는지 컴파일 타임에 검증됨
		RangeLimitMap.Add(InStrategy, FWFC3DFunctionValidator::MakeRangeLimitFunction(InFunction));
	}
	
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
