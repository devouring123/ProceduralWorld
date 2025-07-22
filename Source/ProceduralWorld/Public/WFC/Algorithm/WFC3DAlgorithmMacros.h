// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

enum class ECollapseCellSelectStrategy : uint8;
enum class ECollapseTileInfoSelectStrategy : uint8;
enum class ECollapseSingleCellStrategy : uint8;
enum class ERangeLimitStrategy : uint8;

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
 * Propagation Strategy Declaration Macros (For Header Files) 
 */
#define DECLARE_PROPAGATOR_RANGE_LIMIT_STRATEGY(StrategyName) \
    extern ERangeLimitStrategy StrategyName##_Enum; \
    bool StrategyName(const FIntVector& CollapseLocation, const FIntVector& PropagationLocation, const int32 RangeLimit)


/**
 * Collapse Strategy Implementation Macros (For Source Files)
 */
#define IMPLEMENT_COLLAPSER_CELL_SELECTOR_STRATEGY(StrategyName) \
    int32 StrategyName##CellSelector(const FWFC3DCollapseContext& Context); \
    namespace { \
        struct FRegister_##StrategyName##_CellSelector \
        { \
            FRegister_##StrategyName##_CellSelector() \
            { \
                FCoreDelegates::OnPostEngineInit.AddLambda([]() \
                { \
                    FWFC3DFunctionMaps::RegisterCellSelectorEnum(ECollapseCellSelectStrategy::StrategyName, StrategyName##CellSelector); \
                }); \
            } \
        }; \
        static FRegister_##StrategyName##_CellSelector Register_##StrategyName##_Instance; \
    } \
    int32 StrategyName##CellSelector(const FWFC3DCollapseContext& Context)

#define IMPLEMENT_COLLAPSER_TILE_SELECTOR_STRATEGY(StrategyName) \
    const FTileInfo* StrategyName##TileSelector(const FWFC3DCollapseContext& Context, const int32 SelectedCellIndex); \
    namespace \
    { \
        struct FRegister_##StrategyName##_TileSelector \
        { \
            FRegister_##StrategyName##_TileSelector() \
            { \
                FCoreDelegates::OnPostEngineInit.AddLambda([](){ \
                    FWFC3DFunctionMaps::RegisterTileInfoSelectorEnum(ECollapseTileInfoSelectStrategy::StrategyName, StrategyName##TileSelector); \
                }); \
            } \
        }; \
        static FRegister_##StrategyName##_TileSelector Register_##StrategyName##_Instance; \
    } \
    const FTileInfo* StrategyName##TileSelector(const FWFC3DCollapseContext& Context, const int32 SelectedCellIndex)
    
#define IMPLEMENT_COLLAPSER_CELL_COLLAPSER_STRATEGY(StrategyName) \
    bool StrategyName##CellCollapser(FWFC3DCell* SelectedCell, int32 SelectedCellIndex, const FTileInfo* SelectedTileInfo); \
    namespace \
    { \
        struct FRegister_##StrategyName##_CellCollapser \
        { \
            FRegister_##StrategyName##_CellCollapser() \
            { \
                FCoreDelegates::OnPostEngineInit.AddLambda([](){ \
                    FWFC3DFunctionMaps::RegisterCellCollapserEnum(ECollapseSingleCellStrategy::StrategyName, StrategyName##CellCollapser); \
                }); \
            } \
        }; \
        static FRegister_##StrategyName##_CellCollapser Register_##StrategyName##_Instance; \
    } \
    bool StrategyName##CellCollapser(FWFC3DCell* SelectedCell, int32 SelectedCellIndex, const FTileInfo* SelectedTileInfo)

        

#define IMPLEMENT_PROPAGATOR_RANGE_LIMIT_STRATEGY(StrategyName) \
    bool StrategyName##RangeLimit(const FIntVector& CollapseLocation, const FIntVector& PropagationLocation, const int32 RangeLimit); \
    namespace \
    { \
        struct FRegister_##StrategyName##_RangeLimit \
        { \
            FRegister_##StrategyName##_RangeLimit() \
            { \
                FCoreDelegates::OnPostEngineInit.AddLambda([](){ \
                    FWFC3DFunctionMaps::RegisterRangeLimitEnum(ERangeLimitStrategy::StrategyName, StrategyName##RangeLimit); \
                }); \
            } \
        }; \
        static FRegister_##StrategyName##_RangeLimit Register_##StrategyName##_Instance; \
    } \
    bool StrategyName##RangeLimit(const FIntVector& CollapseLocation, const FIntVector& PropagationLocation, const int32 RangeLimit)
