// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

enum class ECollapseCellSelectStrategy : uint8;
enum class ECollapseTileInfoSelectStrategy : uint8;
enum class ECollapseSingleCellStrategy : uint8;

struct FWFC3DCell;
struct FRandomStream;
struct FTileInfo;

class UWFC3DGrid;
class UWFC3DModelDataAsset;
class FWFC3DFunctionMaps;

/**
 * Collapse Strategy Declaration Macros (For Header Files)
 */
#define DECLARE_COLLAPSER_CELL_SELECTOR_STRATEGY(StrategyName) \
    extern ECollapseCellSelectStrategy StrategyName##_Enum; \
    int32 StrategyName(const FWFC3DCollapseContext& Context)

#define DECLARE_COLLAPSER_TILE_SELECTOR_STRATEGY(StrategyName) \
    extern ECollapseTileInfoSelectStrategy StrategyName##_Enum; \
    const FTileInfo* StrategyName(const FWFC3DCollapseContext& Context, const int32 SelectedCellIndex)

#define DECLARE_COLLAPSER_CELL_COLLAPSER_STRATEGY(StrategyName) \
    extern ECollapseSingleCellStrategy StrategyName##_Enum; \
    bool StrategyName(FWFC3DCell* SelectedCell, int32 SelectedCellIndex, const FTileInfo* SelectedTileInfo)

/**
 * Collapse Strategy Implementation Macros (For Source Files)
 */
#define IMPLEMENT_COLLAPSER_CELL_SELECTOR_STRATEGY(StrategyName) \
    ECollapseCellSelectStrategy StrategyName##_Enum = ECollapseCellSelectStrategy::StrategyName; \
    struct FRegister_##StrategyName##_CellSelector \
    { \
        FRegister_##StrategyName##_CellSelector() \
        { \
            FWFC3DFunctionMaps::RegisterCellSelectorEnum(StrategyName##_Enum, StrategyName); \
        } \
    }; \
    static FRegister_##StrategyName##_CellSelector Register_##StrategyName##_Instance; \
    int32 StrategyName(const FWFC3DCollapseContext& Context)

#define IMPLEMENT_COLLAPSER_TILE_SELECTOR_STRATEGY(StrategyName) \
    ECollapseTileInfoSelectStrategy StrategyName##_Enum = ECollapseTileInfoSelectStrategy::StrategyName; \
    struct FRegister_##StrategyName##_TileSelector \
    { \
        FRegister_##StrategyName##_TileSelector() \
        { \
            FWFC3DFunctionMaps::RegisterTileInfoSelectorEnum(StrategyName##_Enum, StrategyName); \
        } \
    }; \
    static FRegister_##StrategyName##_TileSelector Register_##StrategyName##_Instance; \
    const FTileInfo* StrategyName(const FWFC3DCollapseContext& Context, const int32 SelectedCellIndex)

#define IMPLEMENT_COLLAPSER_CELL_COLLAPSER_STRATEGY(StrategyName) \
    ECollapseSingleCellStrategy StrategyName##_Enum = ECollapseSingleCellStrategy::StrategyName; \
    struct FRegister_##StrategyName##_CellCollapser \
    { \
        FRegister_##StrategyName##_CellCollapser() \
        { \
            FWFC3DFunctionMaps::RegisterCellCollapserEnum(StrategyName##_Enum, StrategyName); \
        } \
    }; \
    static FRegister_##StrategyName##_CellCollapser Register_##StrategyName##_Instance; \
    bool StrategyName(FWFC3DCell* SelectedCell, const int32 SelectedCellIndex, const FTileInfo* SelectedTileInfo)

