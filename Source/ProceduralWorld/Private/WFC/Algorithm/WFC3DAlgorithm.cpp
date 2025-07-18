// Copyright Epic Games, Inc. All Rights Reserved.

#include "WFC/Algorithm/WFC3DAlgorithm.h"
#include "WFC/Algorithm/WFC3DAlgorithmTypes.h"
#include "WFC/Algorithm/WFC3DCollapse.h"
#include "WFC/Algorithm/WFC3DPropagation.h"
#include "WFC/Algorithm/WFC3DFunctionMaps.h"
#include "WFC/Data/WFC3DGrid.h"

FWFC3DResult UWFC3DAlgorithm::Execute(const FWFC3DAlgorithmContext& Context)
{
	FWFC3DResult Result;

	UWFC3DGrid* Grid = Context.Grid;
	const UWFC3DModelDataAsset* ModelData = Context.ModelData;

	if (Grid == nullptr)
	{
		UE_LOG(LogTemp, Error, TEXT("Invalid Grid in Algorithm Context"));
		return Result;
	}

	if (ModelData == nullptr)
	{
		UE_LOG(LogTemp, Error, TEXT("Invalid ModelData in Algorithm Context"));
		return Result;
	}

	// Collapse Context 생성
	FWFC3DCollapseContext CollapseContext(Grid, ModelData, &RandomStream);

	// Collapse 함수 포인터 획득
	SelectCellFunc SelectCellFuncPtr = FWFC3DFunctionMaps::GetCellSelectorFunction(CollapseStrategy.CellSelectStrategy);
	SelectTileInfoFunc SelectTileInfoFuncPtr = FWFC3DFunctionMaps::GetTileInfoSelectorFunction(CollapseStrategy.TileSelectStrategy);
	CollapseSingleCellFunc CollapseSingleCellFuncPtr = FWFC3DFunctionMaps::GetCellCollapserFunction(CollapseStrategy.CellCollapseStrategy);

	if (SelectCellFuncPtr == nullptr || SelectTileInfoFuncPtr == nullptr || CollapseSingleCellFuncPtr == nullptr)
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to get Collapse function pointers"));
		return Result;
	}

	while (!Grid->GetRemainingCells())
	{
		// Collapse 실행
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
			return Result;
		}

		// Propagation Context 생성
		FWFC3DPropagationContext PropagationContext(Grid, ModelData, CollapseResult.CollapsedLocation);

		// Propagation 실행
		FPropagationResult PropagationResult = WFC3DPropagateFunctions::ExecutePropagation(PropagationContext, PropagationStrategy);

		Result.PropagationResults.Add(PropagationResult);

		if (!PropagationResult.bSuccess)
		{
			UE_LOG(LogTemp, Error, TEXT("Propagation failed"));
			return Result;
		}

		UE_LOG(LogTemp, Display, TEXT("WFC3D Algorithm executed successfully. Collapsed at %s, Affected %d cells"),
		       *CollapseResult.CollapsedLocation.ToString(),
		       PropagationResult.AffectedCellCount);
	}
	
	Result.bSuccess = true;
	
	return Result;
}
