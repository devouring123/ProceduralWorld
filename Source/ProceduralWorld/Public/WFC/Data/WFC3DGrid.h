// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "WFC3DCell.h"
#include "UObject/Object.h"
#include "WFC3DGrid.generated.h"

class UWFC3DModelDataAsset;

/**
 * 
 */
UCLASS(BlueprintType, Blueprintable)
class PROCEDURALWORLD_API UWFC3DGrid : public UObject
{
	GENERATED_BODY()

public:
	UWFC3DGrid()
	{
		// Grid 초기화
		WFC3DCells.Init(FWFC3DCell(), Dimension.X * Dimension.Y * Dimension.Z);
		RemainingCells = Dimension.X * Dimension.Y * Dimension.Z;
		
		UE_LOG(LogTemp, Log, TEXT("WFC3DGrid Default Constructor - Dimension: %s, RemainingCells: %d"), 
			*Dimension.ToString(), RemainingCells);
	}

	/** Grid를 특정 크기로 초기화하는 함수 */
	UFUNCTION(BlueprintCallable, Category = "WFC3D")
	void InitializeGrid(const FIntVector& InDimension, const UWFC3DModelDataAsset* InModelData);
	
	FORCEINLINE TArray<FWFC3DCell>* GetAllCells() { return &WFC3DCells; }

	FWFC3DCell* GetCell(const int32 Index);
	FWFC3DCell* GetCell(const FIntVector& Location);
	FWFC3DCell* GetCell(const int32 X, const int32 Y, const int32 Z);

	FORCEINLINE FIntVector GetDimension() const { return Dimension; }
	FORCEINLINE int32 Num() const { return WFC3DCells.Num(); }

	int32 GetRemainingCells() const;
	void DecreaseRemainingCells();

	FORCEINLINE bool IsValidLocation(const int32 Index) const;
	FORCEINLINE bool IsValidLocation(const FIntVector& Location) const;
	FORCEINLINE bool IsValidLocation(const int32 X, const int32 Y, const int32 Z) const;

	void PrintGridInfo() const
	{
		UE_LOG(LogTemp, Log, TEXT("WFC3DGrid Info - Dimension: %s, Total Cells: %d, Remaining Cells: %d"), 
			*Dimension.ToString(), WFC3DCells.Num(), RemainingCells);
		for (const FWFC3DCell& Cell : WFC3DCells)
		{
			Cell.PrintCellInfo();
		}
	}
	
private:
	UPROPERTY(EditAnywhere, Category = "WFC3D")
	TArray<FWFC3DCell> WFC3DCells;

	UPROPERTY(EditAnywhere, Category = "WFC3D")
	FIntVector Dimension = FIntVector(5,5,5);

	UPROPERTY(EditAnywhere, Category = "WFC3D")
	int32 RemainingCells = 0;
};
