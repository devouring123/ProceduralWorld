#include "WFC/Algorithm/WFC3DCollapse.h"
#include "WFC/Algorithm/WFC3DFunctionMaps.h"
#include "WFC/Data/WFC3DGrid.h"
#include "WFC/Data/WFC3DModelDataAsset.h"

namespace WFC3DCollapseFunctions
{
	FCollapseResult ExecuteCollapse(
		const FWFC3DCollapseContext& Context,
		const SelectCellFunc SelectCellFuncPtr,
		const SelectTileInfoFunc SelectTileInfoFuncPtr,
		const CollapseSingleCellFunc CollapseSingleCellFuncPtr
	)
	{
		FCollapseResult Result;
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
		if (SelectCellFuncPtr == nullptr)
		{
			UE_LOG(LogTemp, Error, TEXT("Failed to get Cell Selector"));
			return Result;
		}
		if (SelectTileInfoFuncPtr == nullptr)
		{
			UE_LOG(LogTemp, Error, TEXT("Failed to get TileInfo Selector"));
			return Result;
		}
		if (CollapseSingleCellFuncPtr == nullptr)
		{
			UE_LOG(LogTemp, Error, TEXT("Failed to get Cell Collapser"));
			return Result;
		}

		TArray<FWFC3DCell>* GridCells = Grid->GetAllCells();

		// Cell 선택
		int32 SelectedCellIndex = SelectCellFuncPtr(Context);
		if (!GridCells->IsValidIndex(SelectedCellIndex))
		{
			UE_LOG(LogTemp, Error, TEXT("Failed to select a cell"));
			return Result;
		}

		FWFC3DCell* SelectedCell = &(*GridCells)[SelectedCellIndex];
		Result.CollapsedIndex = SelectedCellIndex;
		Result.CollapsedLocation = SelectedCell->Location;
		
		// 선택된 Cell의 TileInfo 선택
		const FTileInfo* SelectedTileInfo = SelectTileInfoFuncPtr(Context, SelectedCellIndex);
		if (SelectedTileInfo == nullptr)
		{
			UE_LOG(LogTemp, Error, TEXT("Failed to select a TileInfo"));
			return Result;
		}

		// 선택된 Cell Collapse
		if (!CollapseSingleCellFuncPtr(SelectedCell, SelectedCellIndex, SelectedTileInfo))
		{
			UE_LOG(LogTemp, Error, TEXT("Failed to collapse cell"));
			return Result;
		}

		Result.bSuccess = true;
		return Result;
	}

	namespace CellSelector
	{
		IMPLEMENT_COLLAPSER_CELL_SELECTOR_STRATEGY(ByEntropy)
		{
			UWFC3DGrid* Grid = Context.Grid;
			const FRandomStream* RandomStream = Context.RandomStream;
			if (Grid == nullptr)
			{
				UE_LOG(LogTemp, Error, TEXT("Invalid Grid"));
				return -1;
			}

			TArray<FWFC3DCell>* GridCells = Grid->GetAllCells();
			int32 GridCellsSize = GridCells->Num();

			int32 LowestEntropy = INT32_MAX;
			TArray<int32> CellIndicesWithLowestEntropy;

			/** Find Lowest Entropy */
			for (int32 i = 0; i < GridCellsSize; ++i)
			{
				if ((*GridCells)[i].Entropy < LowestEntropy && !(*GridCells)[i].bIsCollapsed)
				{
					LowestEntropy = (*GridCells)[i].Entropy;
					CellIndicesWithLowestEntropy.Empty();
					CellIndicesWithLowestEntropy.Add(i);
				}
				else if ((*GridCells)[i].Entropy == LowestEntropy && !(*GridCells)[i].bIsCollapsed)
				{
					CellIndicesWithLowestEntropy.Add(i);
				}
			}

			if (LowestEntropy == INT32_MAX)
			{
				UE_LOG(LogTemp, Error, TEXT("No Valid Cells"));
				return -1;
			}

			if (LowestEntropy == 0)
			{
				for (int32 i = 0; i < GridCellsSize; ++i)
				{
					if ((*GridCells)[i].Entropy == LowestEntropy && !(*GridCells)[i].bIsCollapsed)
					{
						UE_LOG(LogTemp, Display, TEXT("Collapse Grid Failed With Lowest Entropy = 0, Index %d"), i);
					}
				}
				return -1;
			}

			if (CellIndicesWithLowestEntropy.Num() == 0)
			{
				UE_LOG(LogTemp, Error, TEXT("No Valid Cells With Lowest Entropy"));
				return -1;
			}

			/** Select Cell From Lowest Entropies */
			int32 SelectedIndexInLowestEntropy = RandomStream->RandRange(0, CellIndicesWithLowestEntropy.Num() - 1);
			return CellIndicesWithLowestEntropy[SelectedIndexInLowestEntropy];
		}

		IMPLEMENT_COLLAPSER_CELL_SELECTOR_STRATEGY(Random)
		{
			UWFC3DGrid* Grid = Context.Grid;
			const FRandomStream* RandomStream = Context.RandomStream;
			if (Grid == nullptr)
			{
				UE_LOG(LogTemp, Error, TEXT("Invalid Grid"));
				return -1;
			}

			TArray<FWFC3DCell>* GridCells = Grid->GetAllCells();
			int32 GridCellsSize = GridCells->Num();

			/** Find UnCollapsed Cell */
			TArray<int32> UnCollapsedCellIndices;
			for (int32 i = 0; i < GridCellsSize; ++i)
			{
				if (!(*GridCells)[i].bIsCollapsed)
				{
					UnCollapsedCellIndices.Add(i);
				}
			}

			if (UnCollapsedCellIndices.Num() == 0)
			{
				UE_LOG(LogTemp, Error, TEXT("No Uncollapsed Cells"));
				return -1;
			}

			/** Select Random Cell */
			int32 RandomIndex = RandomStream->RandRange(0, UnCollapsedCellIndices.Num() - 1);
			return UnCollapsedCellIndices[RandomIndex];
		}

		IMPLEMENT_COLLAPSER_CELL_SELECTOR_STRATEGY(Custom)
		{
			/** Make Your Custom CellSelector */
			return -1;
		}
	}

	namespace TileInfoSelector
	{
		IMPLEMENT_COLLAPSER_TILE_SELECTOR_STRATEGY(ByWeight)
		{
			UWFC3DGrid* Grid = Context.Grid;
			const UWFC3DModelDataAsset* ModelData = Context.ModelData;
			const FRandomStream* RandomStream = Context.RandomStream;
			if (Grid == nullptr || ModelData == nullptr || SelectedCellIndex < 0)
			{
				UE_LOG(LogTemp, Error, TEXT("Invalid Parameters for SelectTileInfoByWeight"));
				return nullptr;
			}

			TArray<FWFC3DCell>* GridCells = Grid->GetAllCells();
			if (!GridCells->IsValidIndex(SelectedCellIndex))
			{
				UE_LOG(LogTemp, Error, TEXT("Invalid Cell Index"));
				return nullptr;
			}

			/** Get Enalbe TileInfos */
			TArray<int32> TileInfoIndices = FWFC3DHelperFunctions::GetAllIndexFromBitset((*GridCells)[SelectedCellIndex].RemainingTileOptionsBitset);
			if (TileInfoIndices.Num() == 0)
			{
				UE_LOG(LogTemp, Error, TEXT("No Valid TileInfo Indices"));
				return nullptr;
			}

			/** Get Weights */
			TArray<float> Weights;
			for (int32 Index : TileInfoIndices)
			{
				Weights.Add(ModelData->GetTileInfo(Index)->Weight);
			}

			/** Select By Weights */
			int32 SelectedTileInfoIndex = FWFC3DHelperFunctions::GetWeightedRandomIndex(Weights, RandomStream);
			return ModelData->GetTileInfo(TileInfoIndices[SelectedTileInfoIndex]);
		}

		IMPLEMENT_COLLAPSER_TILE_SELECTOR_STRATEGY(Random)
		{
			UWFC3DGrid* Grid = Context.Grid;
			const UWFC3DModelDataAsset* ModelData = Context.ModelData;
			const FRandomStream* RandomStream = Context.RandomStream;
			if (Grid == nullptr || ModelData == nullptr || SelectedCellIndex < 0)
			{
				UE_LOG(LogTemp, Error, TEXT("Invalid Parameters for SelectTileInfoRandom"));
				return nullptr;
			}

			TArray<FWFC3DCell>* GridCells = Grid->GetAllCells();
			if (!GridCells->IsValidIndex(SelectedCellIndex))
			{
				UE_LOG(LogTemp, Error, TEXT("Invalid Cell Index"));
				return nullptr;
			}

			/** Get Enalbe TileInfos */
			TArray<int32> TileInfoIndices = FWFC3DHelperFunctions::GetAllIndexFromBitset((*GridCells)[SelectedCellIndex].RemainingTileOptionsBitset);
			if (TileInfoIndices.Num() == 0)
			{
				UE_LOG(LogTemp, Error, TEXT("No Valid TileInfo Indices"));
				return nullptr;
			}

			/** Select Randomly */
			int32 RandomIndex = RandomStream->RandRange(0, TileInfoIndices.Num() - 1);
			return ModelData->GetTileInfo(TileInfoIndices[RandomIndex]);
		}

		IMPLEMENT_COLLAPSER_TILE_SELECTOR_STRATEGY(Custom)
		{
			/** Make Your Custom TileInfoSelector */
			return nullptr;
		}
	}

	namespace CellCollapser
	{
		IMPLEMENT_COLLAPSER_CELL_COLLAPSER_STRATEGY(Default)
		{
			if (SelectedCell == nullptr)
			{
				UE_LOG(LogTemp, Error, TEXT("Invalid Cell"));
				return false;
			}
			if (SelectedTileInfo == nullptr)
			{
				UE_LOG(LogTemp, Error, TEXT("Invalid TileInfo"));
				return false;
			}
			if (SelectedCell->bIsCollapsed)
			{
				UE_LOG(LogTemp, Error, TEXT("Cell is already collapsed"));
				return false;
			}

			SelectedCell->CollapsedTileInfo = SelectedTileInfo;
			SelectedCell->Entropy = 1;
			SelectedCell->bIsCollapsed = true;
			SelectedCell->RemainingTileOptionsBitset.Init(false, SelectedCell->RemainingTileOptionsBitset.Num());
			SelectedCell->RemainingTileOptionsBitset[SelectedCellIndex] = true;

			const TArray<int32>& FacesIndices = SelectedTileInfo->Faces;
			for (const EFace& Direction : FWFC3DFaceUtils::AllDirections)
			{
				uint8 DirectionIndex = FWFC3DFaceUtils::GetIndex(Direction);
				SelectedCell->MergedFaceOptionsBitset[DirectionIndex].Init(false, SelectedCell->MergedFaceOptionsBitset[DirectionIndex].Num());
				SelectedCell->MergedFaceOptionsBitset[DirectionIndex][FacesIndices[DirectionIndex]] = true;
			}

			SelectedCell->bIsCollapsed = true;
			SelectedCell->bIsPropagated = true;

			return true;
		}

		IMPLEMENT_COLLAPSER_CELL_COLLAPSER_STRATEGY(Custom)
		{
			/** Make Your Custom CellCollapser */
			return true;
		}
	}
}
