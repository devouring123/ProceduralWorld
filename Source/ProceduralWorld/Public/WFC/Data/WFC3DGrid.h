// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "WFC3DCell.h"
#include "UObject/Object.h"
#include "WFC3DGrid.generated.h"

/**
 * 
 */
UCLASS(BlueprintType, Blueprintable)
class PROCEDURALWORLD_API UWFC3DGrid : public UObject
{
	GENERATED_BODY()

public:
	UWFC3DGrid() = default;

	UWFC3DGrid(const FIntVector& InDimension): Dimension(InDimension)
	{
		/** Initialize the Grid with Dimension */
		WFC3DCells.Init(FWFC3DCell(), Dimension.X * Dimension.Y * Dimension.Z);
		RemainingCells = Dimension.X * Dimension.Y * Dimension.Z;
	}

	FORCEINLINE TArray<FWFC3DCell>* GetAllCells() { return &WFC3DCells; }

	FWFC3DCell* GetCell(const int32 Index);
	FWFC3DCell* GetCell(const FIntVector& Location);
	FWFC3DCell* GetCell(const int32 X, const int32 Y, const int32 Z);

	FIntVector GetDimension() const { return Dimension; }
	int32 GetRemainingCells() const;
	int32 Num() const { return WFC3DCells.Num(); }

private:
	UPROPERTY(EditAnywhere, Category = "WFC3D")
	TArray<FWFC3DCell> WFC3DCells;

	UPROPERTY(EditAnywhere, Category = "WFC3D")
	FIntVector Dimension;

	UPROPERTY(EditAnywhere, Category = "WFC3D")
	int32 RemainingCells = 0;

	UPROPERTY(EditAnywhere, Category = "WFC3D")
	bool bChangedRemainingCells = false;
	
	bool IsValidLocation(const int32 Index) const;
	bool IsValidLocation(const FIntVector& Location) const;
	bool IsValidLocation(const int32 X, const int32 Y, const int32 Z) const;
};
