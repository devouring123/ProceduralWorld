// Fill out your copyright notice in the Description page of Project Settings.


#include "WFC/Algorithm/WFC3DCollapse.h"
#include "WFC/Data/WFC3DGrid.h"
#include "WFC/Data/WFC3DModelDataAsset.h"


bool WFC3DCollapseFunctions::CollapseCell(FWFC3DCell* SelectedCell, const FTileInfo* SelectedTileInfo)
{
	return true;
}

FCollapseResult WFC3DCollapseFunctions::StandardCollapse(UWFC3DGrid* Grid, const UWFC3DModelDataAsset* ModelData, FRandomStream& RandomStream)
{
	FCollapseResult Result;

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

	int32 LowestEntropy = INT32_MAX;
	for (const FWFC3DCell& Cell : *Grid->GetAllCells())
	{
		if (Cell.Entropy < LowestEntropy && !Cell.bIsCollapsed)
		{
			LowestEntropy = Cell.Entropy;
		}
	}

	if (LowestEntropy == INT32_MAX)
	{
		UE_LOG(LogTemp, Error, TEXT("No Valid Cells"));
		return Result;
	}

	if (LowestEntropy == 0)
	{
		TArray<int32> CellsWithLowestEntropy;
		for (const FWFC3DCell& Cell : *Grid->GetAllCells())
		{
			if (Cell.Entropy == LowestEntropy && !Cell.bIsCollapsed)
			{
				UE_LOG(LogTemp, Display, TEXT("Collapse Grid Failed With Lowest Entropy = 0, Index %d"),
				       Cell.Location.X + Cell.Location.Y * Grid->GetDimension().X + Cell.Location.Z * Grid->GetDimension().X * Grid->GetDimension().Y);
			}
		}
		return Result;
	}

	TArray<FWFC3DCell*> CellsWithLowestEntropy;
	for (FWFC3DCell& Cell : *Grid->GetAllCells())
	{
		if (Cell.Entropy == LowestEntropy && !Cell.bIsCollapsed)
		{
			CellsWithLowestEntropy.Add(&Cell);
		}
	}

	if (CellsWithLowestEntropy.Num() == 0)
	{
		UE_LOG(LogTemp, Error, TEXT("No Valid Cells With Lowest Entropy"));
		return Result;
	}
	
	int32 SelectedCellIndex = RandomStream.RandRange(0, CellsWithLowestEntropy.Num() - 1);
	FWFC3DCell* SelectedCell = CellsWithLowestEntropy[SelectedCellIndex];

	TArray<int32> TileInfoIndex = WFC3DHelperFunctions::GetAllIndexFromBitset(SelectedCell->RemainingTileOptionsBitset);
	if (TileInfoIndex.Num() == 0)
	{
		UE_LOG(LogTemp, Error, TEXT("No Valid TileInfo Index"));
		return Result;
	}

	TArray<float> Weights;
	for (int32 Index : TileInfoIndex)
	{
		Weights.Add(ModelData->GetTileInfo(Index)->Weight);
	}
	int32 SelectedTileInfoIndex = WFC3DHelperFunctions::GetWeightedRandomIndex(Weights, RandomStream);
	CollapseCell(SelectedCell, ModelData->GetTileInfo(TileInfoIndex[SelectedTileInfoIndex]));

	//TODO:: CollapseCell
	
	return Result;
}

CollapseFunc WFC3DCollapseFunctions::GetCollapseFunction(ECollapseStrategy Strategy)
{
	switch (Strategy)
	{
	case ECollapseStrategy::Standard:
		return StandardCollapse;
	case ECollapseStrategy::OnlyWeighted:
		// return OnlyWeightedCollapse;
	case ECollapseStrategy::OnlyRandom:
		// return OnlyRandomCollapse;
	case ECollapseStrategy::Custom:
		// return CustomCollapse;
	default:
		return nullptr;
	}
}
