// Fill out your copyright notice in the Description page of Project Settings.


#include "WFC/Algorithm/WFC3DPropagation.h"

#include "WFC/Algorithm/WFC3DFunctionMaps.h"
#include "WFC/Data/WFC3DFaceUtils.h"
#include "WFC/Data/WFC3DGrid.h"
#include "WFC/Data/WFC3DModelDataAsset.h"

namespace WFC3DPropagateFunctions
{
	FPropagationResult ExecutePropagation(const FWFC3DPropagationContext& Context, const FPropagationStrategy& PropagationStrategy)
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
			UE_LOG(LogTemp, Error, TEXT("No Remaining Cells In Propagation"));
			return Result;
		}

		for (FWFC3DCell& Cell : *Grid->GetAllCells())
		{
			// 붕괴하지 않은 모든 셀의 bIsPropagated 플래그를 false로 초기화
			if (!Cell.bIsCollapsed)
			{
				Cell.bIsPropagated = false;
			}
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
			CellToPropagate->SetPropagatedFaces(FWFC3DFaceUtils::GetOpposite(Direction));
			PropagationQueue.Enqueue(PropagationLocation);
		}

		// Range Limit 함수
		RangeLimitFunc RangeLimitFuncPtr = nullptr;
		if (PropagationStrategy.RangeLimitStrategy != ERangeLimitStrategy::Disable && Context.RangeLimit != 0)
		{
			RangeLimitFuncPtr = FWFC3DFunctionMaps::GetRangeLimitFunction(PropagationStrategy.RangeLimitStrategy);
		}

		// Propagation
		while (!PropagationQueue.IsEmpty())
		{
			FIntVector PropagationLocation;
			PropagationQueue.Dequeue(PropagationLocation);
			FWFC3DCell* PropagatedCell = Grid->GetCell(PropagationLocation);

			// 전파할 셀이 유효하지 않거나 이미 붕괴되었거나 전파된 경우 건너뜀
			if (PropagatedCell == nullptr || PropagatedCell->bIsCollapsed || PropagatedCell->bIsPropagated)
			{
				continue;
			}
			// Range Limit 체크
			if (RangeLimitFuncPtr != nullptr && !RangeLimitFuncPtr(CollapseLocation, PropagationLocation, Context.RangeLimit))
			{
				continue;
			}

			// 단일 셀 전파 함수 호출
			if (PropagateCell(PropagatedCell, Grid, PropagationQueue, ModelData))
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
		Result.bSuccess = true;
		return Result;
	}

	FPropagationResult ExecuteInitialPropagation(const FWFC3DPropagationContext& Context)
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

		for (FWFC3DCell& Cell : *Grid->GetAllCells())
		{
			// 붕괴하지 않은 모든 셀의 bIsPropagated 플래그를 false로 초기화
			if (!Cell.bIsCollapsed)
			{
				Cell.bIsPropagated = false;
			}
		}

		TQueue<FIntVector> PropagationQueue;
		
		// Grid의 각 꼭지점 8개 넣으면 됨
		// 전파 방향은 각 꼭지점 => 바깥 3면에서 전파 받기
		TArray<FWFC3DCell*> CornerCells;
		CornerCells.Add(Grid->GetCell(0,0,0));
		CornerCells.Add(Grid->GetCell(0,0,Grid->GetDimension().Z - 1));
		CornerCells.Add(Grid->GetCell(0,Grid->GetDimension().Y - 1,0));
		CornerCells.Add(Grid->GetCell(0,Grid->GetDimension().Y - 1,Grid->GetDimension().Z - 1));
		CornerCells.Add(Grid->GetCell(Grid->GetDimension().X - 1,0,0));
		CornerCells.Add(Grid->GetCell(Grid->GetDimension().X - 1,0,Grid->GetDimension().Z - 1));
		CornerCells.Add(Grid->GetCell(Grid->GetDimension().X - 1,Grid->GetDimension().Y - 1,0));
		CornerCells.Add(Grid->GetCell(Grid->GetDimension().X - 1,Grid->GetDimension().Y - 1,Grid->GetDimension().Z - 1));
		
		for (FWFC3DCell* Cell : CornerCells)
		{
			PropagationQueue.Enqueue(Cell->Location);
			
			// 각 꼭지점 셀의 3면에 대해서 전파 받기
			for (const EFace& Direction : FWFC3DFaceUtils::AllDirections)
			{
				FIntVector PropagationLocation = Cell->Location + FWFC3DFaceUtils::GetDirectionVector(Direction);
				FWFC3DCell* CellToPropagate = Grid->GetCell(PropagationLocation);
				if (CellToPropagate == nullptr)
				{
					Cell->SetPropagatedFaces(Direction);
					PropagationQueue.Enqueue(PropagationLocation);
				}
			}
		}

		while (!PropagationQueue.IsEmpty())
		{
			FIntVector PropagationLocation;
			if (!PropagationQueue.Dequeue(PropagationLocation))
			{
				continue;
			}
			FWFC3DCell* PropagatedCell = Grid->GetCell(PropagationLocation);

			if (PropagatedCell == nullptr || PropagatedCell->bIsCollapsed || PropagatedCell->bIsPropagated)
			{
				continue;
			}

			if (PropagateCell(PropagatedCell, Grid, PropagationQueue, ModelData))
			{
				Result.AffectedCellCount++;
				PropagatedCell->bIsPropagated = true;
				Result.bSuccess = true;
				UE_LOG(LogTemp, Display, TEXT("Initial Propagation at Location: %s"), *PropagationLocation.ToString());
			}
			else
			{
				Result.bSuccess = false;
				return Result;
			}
		}
		
		Result.bSuccess = true;
		return Result;
	}

	bool PropagateCell(FWFC3DCell* PropagatedCell, UWFC3DGrid* Grid, TQueue<FIntVector>& PropagationQueue,
	                   const UWFC3DModelDataAsset* ModelData)
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

		// 전파 받은 면에 대해서 남은 타일 옵션을 가져와서 병합 -> 남은 타일 몹션에 대해서 전파 받지 않은 방향으로 전파
		// 전파 받은 면에 대하여 전파 받은 면의 타일 옵션을 병합
		for (const EFace Direction : FWFC3DFaceUtils::AllDirections)
		{
			FIntVector NextLocation = PropagatedCell->Location + FWFC3DFaceUtils::GetDirectionVector(Direction);
			// 전파 받은 면이 아닌 경우에 가장 바깥이 아니라면 건너뜀
			if (!PropagatedCell->IsFacePropagated(Direction) && Grid->IsValidLocation(NextLocation))
			{
				continue;
			}

			// 전파 받은 면에 대해서 해당 면을 가질 수 있는 모든 타일 셋들을 OR로 병합
			TBitArray<> MergedTileOptionsForDirection = TBitArray<>(false, ModelData->GetTileInfosNum());

			// 전파 받은 면이 실제로 존재하는 경우
			const FWFC3DCell* NextCell = Grid->GetCell(NextLocation);
			if (NextCell != nullptr)
			{
				TArray<int32> PropagatedFaceOptionIndices = FWFC3DHelperFunctions::GetAllIndexFromBitset(
					NextCell->MergedFaceOptionsBitset[FWFC3DFaceUtils::GetOppositeIndex(Direction)]);

				for (int32 FaceIndex : PropagatedFaceOptionIndices)
				{
					// 해당 타일 옵션과 OR 연산
					const TBitArray<>* FaceIndexBitArray = ModelData->GetCompatibleTiles(FaceIndex);
					MergedTileOptionsForDirection.CombineWithBitwiseOR(*FaceIndexBitArray, EBitwiseOperatorFlags::MaintainSize);
				}
			}
			// 존재하지 않는 경우에는 OuterCell에서 해당 면의 타일 옵션을 가져옴
			else
			{
				int32 OppositeIndex = FWFC3DFaceUtils::GetOppositeIndex(Direction);
				const TBitArray<>* OuterCellTileOptions = ModelData->GetCompatibleTiles(ModelData->GetTileInfo(0)->Faces[OppositeIndex]);
				if (OuterCellTileOptions == nullptr)
				{
					UE_LOG(LogTemp, Error, TEXT("OuterCellTileOptions is null for Direction: %s"),
					       *FWFC3DFaceUtils::GetDirectionVector(Direction).ToString());
					return false;
				}
				// TODO: OuterCellTileInfo를 ModelData에 지정하기
				// 현재는 0번 TileIndex를 OuterCell로 가정
				MergedTileOptionsForDirection.CombineWithBitwiseOR(
					*OuterCellTileOptions,
					EBitwiseOperatorFlags::MaintainSize);
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
			PropagationQueue.Enqueue(NextLocation);
		}

		// 전파 성공
		UE_LOG(LogTemp, Display, TEXT("Successfully propagated cell at Location: %s"), *PropagatedCell->Location.ToString());
		return true;
	}

	namespace RangeLimit
	{
		IMPLEMENT_PROPAGATOR_RANGE_LIMIT_STRATEGY(SphereRangeLimited)
		{
			// CollapseLocation과 PropagationLocation 사이의 거리가 RangeLimit보다 크면 false 반환
			if ((CollapseLocation - PropagationLocation).Size() > RangeLimit)
			{
				return false;
			}
			return true;
		}

		IMPLEMENT_PROPAGATOR_RANGE_LIMIT_STRATEGY(CubeRangeLimited)
		{
			// CollapseLocation과 PropagationLocation의 각 축의 차이가 RangeLimit보다 크면 false 반환
			if (FMath::Abs(CollapseLocation.X - PropagationLocation.X) > RangeLimit ||
				FMath::Abs(CollapseLocation.Y - PropagationLocation.Y) > RangeLimit ||
				FMath::Abs(CollapseLocation.Z - PropagationLocation.Z) > RangeLimit)
			{
				return false;
			}
			return true;
		}
	}
}
