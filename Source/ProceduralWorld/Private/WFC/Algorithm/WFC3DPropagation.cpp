// Fill out your copyright notice in the Description page of Project Settings.


#include "WFC/Algorithm/WFC3DPropagation.h"

#include "WFC/Data/WFC3DFaceUtils.h"
#include "WFC/Data/WFC3DGrid.h"

bool WFC3DPropagateFunctions::PropagateSingleCell(const FIntVector& Location, UWFC3DGrid* Grid, const UWFC3DModelDataAsset* ModelData)
{
	
	return true;
}

FPropagationResult WFC3DPropagateFunctions::StandardPropagate(const FWFC3DPropagationContext& Context, const FPropagationStrategy&, const int32 RangeLimit)
{
	FPropagationResult Result;
	UWFC3DGrid* Grid = Context.Grid;
	const UWFC3DModelDataAsset* ModelData = Context.ModelData;
	const FRandomStream* RandomStream = Context.RandomStream;
	
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
	
	/** Propagation */
	TQueue<FIntVector> PropagationQueue;
	const FIntVector& CollapseLocation = Context.CollapseLocation;
	const FIntVector& Dimension = Grid->GetDimension();
	
	for (const EFace& Direction : FWFC3DFaceUtils::AllDirections)
	{
		FIntVector PropagationLocation = CollapseLocation + FWFC3DFaceUtils::GetDirectionVector(Direction);
		FWFC3DCell* CellToPropagate = Grid->GetCell(PropagationLocation);
		if (CellToPropagate == nullptr || CellToPropagate->bIsCollapsed || CellToPropagate->bIsPropagated)
		{
			continue;
		}
		PropagationQueue.Enqueue(PropagationLocation);
		CellToPropagate->SetPropagatedFaces(Direction);
	}

	while (!PropagationQueue.IsEmpty())
	{
		FIntVector PropagationLocation;
		if (!PropagationQueue.Dequeue(PropagationLocation))
		{
			break;
		}
		FWFC3DCell* PropagatedCell = Grid->GetCell(PropagationLocation);
		if (PropagatedCell == nullptr || PropagatedCell->bIsCollapsed || PropagatedCell->bIsPropagated)
		{
			continue;
		}
		
		

		
	}	
	
	return Result;	
}

PropagateFunc WFC3DPropagateFunctions::GetPropagateFunction(EPropagationStrategy Strategy)
{
	// switch (Strategy)
	// {
	// case EPropagationStrategy::Standard:
	// 	return StandardPropagate;
	// case EPropagationStrategy::RangeLimited:
	// 	// return RangeLimitedPropagate;
	// case EPropagationStrategy::Custom:
	// 	// return CustomPropagate;
	// default:
	// 	return nullptr;
	// }
	return nullptr;
}
