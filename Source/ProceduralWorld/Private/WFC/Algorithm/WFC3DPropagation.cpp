// Fill out your copyright notice in the Description page of Project Settings.


#include "WFC/Algorithm/WFC3DPropagation.h"

bool WFC3DPropagateFunctions::PropagateCell(const FIntVector& Location, UWFC3DGrid* Grid, const UWFC3DModelDataAsset* ModelData)
{
	
	return true;
}

FPropagationResult WFC3DPropagateFunctions::StandardPropagate(UWFC3DGrid* Grid, const UWFC3DModelDataAsset* ModelData, const FRandomStream& RandomStream, const int32 RangeLimit)
{
	FPropagationResult Result;


	return Result;	
}

PropagateFunc WFC3DPropagateFunctions::GetPropagateFunction(EPropagationStrategy Strategy)
{
	switch (Strategy)
	{
	case EPropagationStrategy::Standard:
		return StandardPropagate;
	case EPropagationStrategy::RangeLimited:
		// return RangeLimitedPropagate;
	case EPropagationStrategy::Custom:
		// return CustomPropagate;
	default:
		return nullptr;
	}
}
