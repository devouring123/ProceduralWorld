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
namespace WFC3DCollapseFunctions
{
	/**
	 * 단일 Cell 붕괴
	 * @param SelectedCell - 붕괴할 Cell
	 * @param SelectedTileInfo - 붕괴할 Cell에 들어갈 TileInfo
	 */
	static bool CollapseCell(FWFC3DCell* SelectedCell, const FTileInfo* SelectedTileInfo);
	
	/**
	 * 엔트로피와 가중치 기반 Collapse
	 */
	static FCollapseResult StandardCollapse(UWFC3DGrid* Grid, const UWFC3DModelDataAsset* ModelData, FRandomStream& RandomStream);

	// /**
	//  * 엔트로피 무시, 가중치 기반 Collapse
	//  */
	// FCollapseResult OnlyWeightedCollapse(UWFC3DGrid* Grid, const UWFC3DModelDataAsset* ModelData, FRandomStream& RandomStream);

	// /**
	//  * 엔트로피와 가중치 무시 전체 랜덤 Collapse
	//  */
	// FCollapseResult OnlyRandomCollapse(UWFC3DGrid* Grid, const UWFC3DModelDataAsset* ModelData, FRandomStream& RandomStream);

	// /**
	//  *  커스텀 Collapse
	//  */
	// FCollapseResult CustomCollapse(UWFC3DGrid* Grid, const UWFC3DModelDataAsset* ModelData, FRandomStream& RandomStream);

	/**
	 * 전략에 따른 Collapse 함수 포인터 반환
	 */
	static CollapseFunc GetCollapseFunction(ECollapseStrategy Strategy);
	
};
