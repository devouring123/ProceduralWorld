// Fill out your copyright notice in the Description page of Project Settings.


#include "WFC/Algorithm/WFC3DPropagation.h"

#include "WFC/Data/WFC3DFaceUtils.h"
#include "WFC/Data/WFC3DGrid.h"
#include "WFC/Data/WFC3DModelDataAsset.h"

bool WFC3DPropagateFunctions::PropagateSingleCell(FWFC3DCell* PropagatedCell, UWFC3DGrid* Grid, const UWFC3DModelDataAsset* ModelData)
{
	if (PropagatedCell == nullptr || Grid == nullptr || ModelData == nullptr)
	{
		UE_LOG(LogTemp, Error, TEXT("Invalid PropagatedCell or Grid or ModelData"));
		return false;
	}
	if (PropagatedCell->bIsCollapsed || PropagatedCell->bIsPropagated)
	{
		UE_LOG(LogTemp, Error, TEXT("Cell already collapsed or propagated at Location: %s"), *PropagatedCell->Location.ToString());
		return false;
	}

	// 전파 받은 면 취합 -> 각 면에 대해서 남은 타일 옵션을 가져와서 병합 -> 남은 타일 몹션에 대해서 전파 받지 않은 방향으로 전파

	// 전파 받은 면에 대하여 전파 받은 면의 타일 옵션을 병합
	for (const EFace& Direction : FWFC3DFaceUtils::AllDirections)
	{
		// TODO: 현재 전파 받은 면만 아니면 전부 병합하는데 전파 받지 않아도 가장 외부에 있으면 아래 과정을 해야함
		// 즉 외부 또한 한 개의 셀로 보고 인식 해야 함 근데 외부 값을 어떻게 받아오지? 외부가 문젠데 진짜
		// 셀이 없는 것 처럼 하면 셀이 없으니까 아무거나 해도 된다 이렇게 이어지는데 이걸 외부 값을 지정해줘야 하는가?
		// = WFC3DModelDataAsset에 OuterCell이 있어야 하는 가?
		
		// 전파 받은 면이 아니라면 건너뜀
		if (!PropagatedCell->IsFacePropagated(Direction))
		{
			continue;
		}
		// 전파 받은 면에 대해서 해당 면을 가질 수 있는 모든 타일 셋들을 OR로 병합
		TBitArray<> MergedTileOptionsForDirection = TBitArray<>(false, ModelData->GetTileInfosNum());
		const TBitArray<>& PropagatedFaceOptions = Grid->GetCell(PropagatedCell->Location + FWFC3DFaceUtils::GetDirectionVector(Direction))
		                                               ->MergedFaceOptionsBitset[FWFC3DFaceUtils::GetOppositeIndex(Direction)];
		for (int32 FaceIndex = 0; FaceIndex < PropagatedFaceOptions.Num(); ++FaceIndex)
		{
			if (!PropagatedFaceOptions[FaceIndex])
			{
				continue;
			}
			// 해당 타일 옵션과 OR 연산
			MergedTileOptionsForDirection.CombineWithBitwiseOR(*ModelData->GetCompatibleTiles(FaceIndex), EBitwiseOperatorFlags::MaintainSize);
		}

		// 전파 받은 면의 타일 옵션을 RemainingTileOptionsBitset과 And 연산
		PropagatedCell->RemainingTileOptionsBitset.CombineWithBitwiseAND(MergedTileOptionsForDirection, EBitwiseOperatorFlags::MaintainSize);
	}

	int RemainingTileOptionsCount = PropagatedCell->RemainingTileOptionsBitset.CountSetBits();

	// 전파 받은 타일 옵션의 개수가 이전과 같다면 전파하지 않음
	if (PropagatedCell->Entropy == RemainingTileOptionsCount)
	{
		UE_LOG(LogTemp, Display, TEXT("No change in remaining tile options at Location: %s"), *PropagatedCell->Location.ToString());
		return true;
	}

	PropagatedCell->Entropy = RemainingTileOptionsCount;

	// 남은 타일 옵션이 없으면 전파 실패
	if (PropagatedCell->Entropy == 0)
	{
		UE_LOG(LogTemp, Error, TEXT("No valid tile options left at Location: %s"), *PropagatedCell->Location.ToString());
		return false;
	}

	
	// MergedFaceOptionBit 초기화
	for (const EFace& Direction : FWFC3DFaceUtils::AllDirections)
	{
		PropagatedCell->MergedFaceOptionsBitset[FWFC3DFaceUtils::GetIndex(Direction)].Init(false, ModelData->GetFaceInfosNum());
	}


	// RemainingTileOptionsBitset에서 남은 타일 옵션을 가져와서 각 면들을 MergedFaceOptionsBitset에 병합
	TArray<int32> TileIndices = FWFC3DHelperFunctions::GetAllIndexFromBitset(PropagatedCell->RemainingTileOptionsBitset);

	for (int32 TileIndex : TileIndices)
	{
		const FTileInfo* TileInfo = ModelData->GetTileInfo(TileIndex);
		if (TileInfo == nullptr)
		{
			continue;
		}

		for (const EFace& Direction : FWFC3DFaceUtils::AllDirections)
		{
			uint8 DirectionIndex = FWFC3DFaceUtils::GetIndex(Direction);
			PropagatedCell->MergedFaceOptionsBitset[DirectionIndex][TileInfo->Faces[DirectionIndex]] = true;
		}
	}

	for (const EFace& Direction : FWFC3DFaceUtils::AllDirections)
	{
		// 전파 받은 면이면 건너뜀
		if (PropagatedCell->IsFacePropagated(Direction))
		{
			continue;
		}

		// 전파 받지 않은 면에 대해서는 해당 방향으로 전파하기
		FIntVector NextLocation = PropagatedCell->Location + FWFC3DFaceUtils::GetDirectionVector(Direction);
		FWFC3DCell* CellToPropagate = Grid->GetCell(NextLocation);
		if (CellToPropagate == nullptr || CellToPropagate->bIsCollapsed || CellToPropagate->bIsPropagated)
		{
			continue;
		}

		// 전파 받을 Cell에 대해서는 반대 방향에서 온 것 이므로 반대 방향에서 전파 받았다고 설정
		CellToPropagate->SetPropagatedFaces(FWFC3DFaceUtils::GetOpposite(Direction));
	}


	return true;
}

FPropagationResult WFC3DPropagateFunctions::StandardPropagate(const FWFC3DPropagationContext& Context, const FPropagationStrategy&, const int32 RangeLimit)
{
	FPropagationResult Result;
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

	// Propagation Queue 초기화
	TQueue<FIntVector> PropagationQueue;
	const FIntVector& CollapseLocation = Context.CollapseLocation;

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

	// Propagation
	while (!PropagationQueue.IsEmpty())
	{
		FIntVector PropagationLocation;
		PropagationQueue.Dequeue(PropagationLocation);
		FWFC3DCell* PropagatedCell = Grid->GetCell(PropagationLocation);
		if (PropagatedCell == nullptr || PropagatedCell->bIsCollapsed || PropagatedCell->bIsPropagated)
		{
			continue;
		}
		if (PropagateSingleCell(PropagatedCell, Grid, ModelData))
		{
			PropagatedCell->bIsPropagated = true;
			UE_LOG(LogTemp, Display, TEXT("Propagated Cell at Location: %s"), *PropagationLocation.ToString());
			++Result.AffectedCellCount;
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("Failed to propagate cell at Location: %s"), *PropagationLocation.ToString());
			Result.bSuccess = false;
			return Result;
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
