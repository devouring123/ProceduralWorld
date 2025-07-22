// Fill out your copyright notice in the Description page of Project Settings.

#include "WFC/Algorithm/WFC3DFunctionMaps.h"
#include "WFC/Algorithm/WFC3DCollapse.h"

/** static 멤버 변수 정의 */
TMap<ECollapseCellSelectStrategy, SelectCellFunc> FWFC3DFunctionMaps::CellSelectorMap;
TMap<ECollapseTileInfoSelectStrategy, SelectTileInfoFunc> FWFC3DFunctionMaps::TileInfoSelectorMap;
TMap<ECollapseSingleCellStrategy, CollapseSingleCellFunc> FWFC3DFunctionMaps::CellCollapserMap;
TMap<ERangeLimitStrategy, RangeLimitFunc> FWFC3DFunctionMaps::RangeLimitMap;

SelectCellFunc FWFC3DFunctionMaps::GetCellSelectorFunction(ECollapseCellSelectStrategy Strategy)
{
    if (CellSelectorMap.Contains(Strategy))
    {
        return CellSelectorMap[Strategy];
    }
    UE_LOG(LogTemp, Warning, TEXT("Cell selector strategy %d not found, using default"), (uint8)Strategy);
    return CellSelectorMap[ECollapseCellSelectStrategy::ByEntropy];
}

SelectTileInfoFunc FWFC3DFunctionMaps::GetTileInfoSelectorFunction(ECollapseTileInfoSelectStrategy Strategy)
{
    if (TileInfoSelectorMap.Contains(Strategy))
    {
        return TileInfoSelectorMap[Strategy];
    }
    UE_LOG(LogTemp, Warning, TEXT("Tile info selector strategy %d not found, using default"), (uint8)Strategy);
    return TileInfoSelectorMap[ECollapseTileInfoSelectStrategy::ByWeight];
}

CollapseSingleCellFunc FWFC3DFunctionMaps::GetCellCollapserFunction(ECollapseSingleCellStrategy Strategy)
{
    if (CellCollapserMap.Contains(Strategy))
    {
        return CellCollapserMap[Strategy];
    }
    UE_LOG(LogTemp, Warning, TEXT("Cell collapser strategy %d not found, using default"), (uint8)Strategy);
    return CellCollapserMap[ECollapseSingleCellStrategy::Default]; 
}

RangeLimitFunc FWFC3DFunctionMaps::GetRangeLimitFunction(ERangeLimitStrategy Strategy)
{
    if (RangeLimitMap.Contains(Strategy))
    {
        return RangeLimitMap[Strategy];       
    }
    UE_LOG(LogTemp, Warning, TEXT("Range limit strategy %d not found, using default"), (uint8)Strategy);
    return RangeLimitMap[ERangeLimitStrategy::Disable];
}

void FWFC3DFunctionMaps::RegisterCellSelectorEnum(ECollapseCellSelectStrategy Enum, SelectCellFunc Function)
{
    CellSelectorMap.Add(Enum, Function);
}

void FWFC3DFunctionMaps::RegisterTileInfoSelectorEnum(ECollapseTileInfoSelectStrategy Enum, SelectTileInfoFunc Function)
{
    TileInfoSelectorMap.Add(Enum, Function);
}

void FWFC3DFunctionMaps::RegisterCellCollapserEnum(ECollapseSingleCellStrategy Enum, CollapseSingleCellFunc Function)
{
    CellCollapserMap.Add(Enum, Function);
}

void FWFC3DFunctionMaps::RegisterRangeLimitEnum(ERangeLimitStrategy Enum, RangeLimitFunc Function)
{
    RangeLimitMap.Add(Enum, Function);
}
