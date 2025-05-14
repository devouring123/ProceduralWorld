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
	for (const auto& Direction: FWFC3DFaceUtils::AllDirections)
	{
		// PropagationQueue.Enqueue();
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
