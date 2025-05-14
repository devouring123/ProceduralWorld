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

	FORCEINLINE FIntVector GetDimension() const { return Dimension; }
	FORCEINLINE int32 Num() const { return WFC3DCells.Num(); }

	int32 GetRemainingCells() const;
	void DecreaseRemainingCells();

private:
	UPROPERTY(EditAnywhere, Category = "WFC3D")
	TArray<FWFC3DCell> WFC3DCells;

	UPROPERTY(EditAnywhere, Category = "WFC3D")
	FIntVector Dimension;

	UPROPERTY(EditAnywhere, Category = "WFC3D")
	int32 RemainingCells = 0;
	
	bool IsValidLocation(const int32 Index) const;
	bool IsValidLocation(const FIntVector& Location) const;
	bool IsValidLocation(const int32 X, const int32 Y, const int32 Z) const;
};
