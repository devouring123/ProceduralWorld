// Fill out your copyright notice in the Description page of Project Settings.

#include "WFC/Data/WFC3DGrid.h"
#include "WFC/Data/WFC3DCell.h"

void UWFC3DGrid::InitializeGrid(const FIntVector& InDimension)
{
	Dimension = InDimension;
	WFC3DCells.Init(FWFC3DCell(), Dimension.X * Dimension.Y * Dimension.Z);
	RemainingCells = Dimension.X * Dimension.Y * Dimension.Z;
	
	UE_LOG(LogTemp, Log, TEXT("Grid Initialized - Dimension: %s, Total Cells: %d, Remaining Cells: %d"), 
		*Dimension.ToString(), WFC3DCells.Num(), RemainingCells);
}

void UWFC3DGrid::InitializeGrid()
{
	InitializeGrid(FIntVector(5, 5, 5));
}

FWFC3DCell* UWFC3DGrid::GetCell(const int32 Index)
{
	if (!IsValidLocation(Index))
	{
		return nullptr;
	}
	return &WFC3DCells[Index];
}

FWFC3DCell* UWFC3DGrid::GetCell(const FIntVector& Location)
{
	if (!IsValidLocation(Location))
	{
		return nullptr;
	}
	return &WFC3DCells[Location.X + Location.Y * Dimension.X + Location.Z * Dimension.X * Dimension.Y];
}

FWFC3DCell* UWFC3DGrid::GetCell(const int32 X, const int32 Y, const int32 Z)
{
	if (!IsValidLocation(X, Y, Z))
	{
		return nullptr;
	}
	return &WFC3DCells[X + Y * Dimension.X + Z * Dimension.X * Dimension.Y];
}

int32 UWFC3DGrid::GetRemainingCells() const
{
	return RemainingCells;
}

void UWFC3DGrid::DecreaseRemainingCells()
{
	--RemainingCells;
}

bool UWFC3DGrid::IsValidLocation(const int32 Index) const
{
	return WFC3DCells.IsValidIndex(Index);
}

bool UWFC3DGrid::IsValidLocation(const FIntVector& Location) const
{
	return Location.X >= 0 && Location.Y >= 0 && Location.Z >= 0 && Location.X < Dimension.X && Location.Y < Dimension.Y && Location.Z < Dimension.Z;
}

bool UWFC3DGrid::IsValidLocation(const int32 X, const int32 Y, const int32 Z) const
{
	return X >= 0 && X < Dimension.X && Y >= 0 && Y < Dimension.Y && Z >= 0 && Z < Dimension.Z;
}
