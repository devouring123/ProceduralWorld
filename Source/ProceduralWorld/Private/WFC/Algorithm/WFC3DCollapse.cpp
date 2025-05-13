#include "WFC/Algorithm/WFC3DCollapse.h"

#include "WFC/Data/WFC3DGrid.h"
#include "WFC/Data/WFC3DModelDataAsset.h"

namespace WFC3DCollapseFunctions
{
    namespace CellSelector
    {
        int32 ByEntropy(UWFC3DGrid* Grid, const FRandomStream& RandomStream)
        {
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
            int32 SelectedIndexInLowestEntropy = RandomStream.RandRange(0, CellIndicesWithLowestEntropy.Num() - 1);
            return CellIndicesWithLowestEntropy[SelectedIndexInLowestEntropy];
        }

        int32 Random(UWFC3DGrid* Grid, const FRandomStream& RandomStream)
        {
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
            int32 RandomIndex = RandomStream.RandRange(0, UnCollapsedCellIndices.Num() - 1);
            return UnCollapsedCellIndices[RandomIndex];
        }

        int32 Custom(UWFC3DGrid* Grid, const FRandomStream& RandomStream)
        {
            /** Make Your Custom CellSelector */
            return -1;
        }
    }

    namespace TileInfoSelector
    {
        const FTileInfo* ByWeight(UWFC3DGrid* Grid, const UWFC3DModelDataAsset* ModelData, const FRandomStream& RandomStream, int32 SelectedCellIndex)
        {
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

        const FTileInfo* Random(UWFC3DGrid* Grid, const UWFC3DModelDataAsset* ModelData, const FRandomStream& RandomStream, int32 SelectedCellIndex)
        {
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
            int32 RandomIndex = RandomStream.RandRange(0, TileInfoIndices.Num() - 1);
            return ModelData->GetTileInfo(TileInfoIndices[RandomIndex]);
        }

        const FTileInfo* Custom(UWFC3DGrid* Grid, const UWFC3DModelDataAsset* ModelData, const FRandomStream& RandomStream, int32 SelectedCellIndex)
        {
            /** Make Your Custom TileInfoSelector */
            return nullptr;
        }
    }

    namespace CellCollapser
    {
        bool Default(FWFC3DCell* SelectedCell, const int32 SelectedCellIndex, const FTileInfo* SelectedTileInfo)
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
                SelectedCell->MergedFaceOptionsBitset[DirectionIndex].Empty();
                SelectedCell->MergedFaceOptionsBitset[DirectionIndex][FacesIndices[DirectionIndex]] = true;
            }

            SelectedCell->bIsCollapsed = true;
            SelectedCell->bIsPropagated = true;
            
            return true;
        }

        bool Custom(FWFC3DCell* SelectedCell, int32 SelectedCellIndex, const FTileInfo* SelectedTileInfo)
        {
            /** Make Your Custom CellCollapser */
            return true;
        }
    }
}

FCollapseResult FCollapseStrategy::ExecuteCollapse(UWFC3DGrid* Grid, const UWFC3DModelDataAsset* ModelData, const FRandomStream& RandomStream) const
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

    /** Cell Select */
    int32 SelectedCellIndex = CellSelectorFunc(Grid, RandomStream);
    if (SelectedCellIndex < 0)
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to select a cell"));
        return Result;
    }

    /** TileInfo Selector */
    const FTileInfo* SelectedTileInfo = TileInfoSelectorFunc(Grid, ModelData, RandomStream, SelectedCellIndex);
    if (SelectedTileInfo == nullptr)
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to select a TileInfo"));
        return Result;
    }

    /** Selected Cell Collapse */
    TArray<FWFC3DCell>* GridCells = Grid->GetAllCells();
    if (!CellCollapserFunc(&(*GridCells)[SelectedCellIndex], SelectedCellIndex, SelectedTileInfo))
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to collapse cell"));
        return Result;
    }

    Grid->DecreaseRemainingCells();

    Result.bSuccess = true;
    Result.SelectedIndex = SelectedCellIndex;

    return Result;
}

FCollapseStrategy UWFC3DCollapseStrategyManager::CreateStandardStrategy()
{
    return FCollapseStrategy(
        WFC3DCollapseFunctions::CellSelector::ByEntropy,
        WFC3DCollapseFunctions::TileInfoSelector::ByWeight,
        WFC3DCollapseFunctions::CellCollapser::Default,
        TEXT("Standard"));
}

FCollapseStrategy UWFC3DCollapseStrategyManager::CreateWeightedStrategy()
{
    return FCollapseStrategy(
        WFC3DCollapseFunctions::CellSelector::Random,
        WFC3DCollapseFunctions::TileInfoSelector::ByWeight,
        WFC3DCollapseFunctions::CellCollapser::Default,
        TEXT("Weighted"));
}

FCollapseStrategy UWFC3DCollapseStrategyManager::CreateRandomStrategy()
{
    return FCollapseStrategy(
        WFC3DCollapseFunctions::CellSelector::Random,
        WFC3DCollapseFunctions::TileInfoSelector::Random,
        WFC3DCollapseFunctions::CellCollapser::Default,
        TEXT("Random"));
}

FCollapseStrategy UWFC3DCollapseStrategyManager::CreateCustomStrategy(
    SelectCellFunc CellSelectorFunc,
    SelectTileInfoFunc TileInfoSelectorFunc,
    CollapseSingleCellFunc CellCollapserFunc,
    const FString& StrategyName)
{
    return FCollapseStrategy(
        CellSelectorFunc,
        TileInfoSelectorFunc,
        CellCollapserFunc,
        StrategyName);
}