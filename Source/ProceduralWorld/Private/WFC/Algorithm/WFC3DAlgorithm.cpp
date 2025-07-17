// Fill out your copyright notice in the Description page of Project Settings.


#include "WFC/Algorithm/WFC3DAlgorithm.h"
#include "WFC/Algorithm/WFC3DCollapse.h"
#include "WFC/Algorithm/WFC3DPropagation.h"
#include "WFC/Algorithm/WFC3DFunctionMaps.h"
#include "WFC/Data/WFC3DGrid.h"
#include "WFC/Data/WFC3DModelDataAsset.h"

UWFC3DAlgorithm::UWFC3DAlgorithm()
{
}

UWFC3DAlgorithm::UWFC3DAlgorithm(const FCollapseStrategy& InCollapseStrategy, const FPropagationStrategy& InPropagationStrategy)
{
}

FWFC3DResult UWFC3DAlgorithm::Execute(const FWFC3DAlgorithmContext& Context)
{
	FWFC3DResult Result;

	UWFC3DGrid* Grid = Context.Grid;
	const UWFC3DModelDataAsset* ModelData = Context.ModelData;

	if (Grid == nullptr)
	{
		UE_LOG(LogTemp, Error, TEXT("Invalid Grid"));
		return Result;
	}
	if (ModelData == nullptr)
	{
		UE_LOG(LogTemp, Error, TEXT("Invalid ModelData"));
		return Result;
	}
	if (Grid->GetRemainingCells() <= 0)
	{
		UE_LOG(LogTemp, Error, TEXT("No Remaining Cells"));
		return Result;
	}

	/** Cell Select Function */
	SelectCellFunc SelectCellFuncPtr = FWFC3DFunctionMaps::GetCellSelectorFunction(CollapseStrategy.CellSelectStrategy);
	if (SelectCellFuncPtr == nullptr)
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to get Cell Selector"));
		return Result;
	}

	/** TileInfo Selector Function */
	SelectTileInfoFunc SelectTileInfoFuncPtr = FWFC3DFunctionMaps::GetTileInfoSelectorFunction(CollapseStrategy.TileSelectStrategy);
	if (SelectTileInfoFuncPtr == nullptr)
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to get TileInfo Selector"));
		return Result;
	}

	/** Selected Cell Collapse Function */
	CollapseSingleCellFunc CollapseSingleCellFuncPtr = FWFC3DFunctionMaps::GetCellCollapserFunction(CollapseStrategy.CellCollapseStrategy);
	if (CollapseSingleCellFuncPtr == nullptr)
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to get Cell Collapser"));
		return Result;
	}

	TArray<FWFC3DCell>* GridCells = Grid->GetAllCells();
	FWFC3DCollapseContext CollapseContext = FWFC3DCollapseContext(Grid, ModelData, &RandomStream);
	// TODO: CollapseResult와 PropagationResult 설정하기
	// 다른 것 들 도 다 설정 해서 Result 생성해야함
	while (Grid->GetRemainingCells() > 0)
	{
		FCollapseResult CollapseResult = WFC3DCollapseFunctions::ExecuteCollapse(CollapseContext, SelectCellFuncPtr, SelectTileInfoFuncPtr, CollapseSingleCellFuncPtr);
		
	}


	return Result;
}
