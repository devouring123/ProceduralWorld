// Fill out your copyright notice in the Description page of Project Settings.


#include "WFC/Data/WFC3DCell.h"

#include "WFC/Data/WFC3DFaceUtils.h"

void FWFC3DCell::Initialize(const int32 TileInfoNum, const int32 FaceInfoNum, const int32 Index, const FIntVector& Dimension)
{
	UE_LOG(LogTemp, Display, TEXT("Index: %d"), Index);

	/** Initialize Common Data */
	bIsCollapsed = false;
	bIsPropagated = false;
	Location = IndexToLocation(Index, Dimension);
	CollapsedTileInfoIndex = INDEX_NONE;
	CollapsedTileInfo = nullptr;

	/** Initialize Algorithm Data */
	Entropy = TileInfoNum;
	PropagatedFaces = 0;
	RemainingTileOptionsBitset.Init(true, TileInfoNum);
	MergedFaceOptionsBitset.SetNum(6);
	for (const EFace& Direction : FWFC3DFaceUtils::AllDirections)
	{
		MergedFaceOptionsBitset[FWFC3DFaceUtils::GetIndex(Direction)].Init(true, FaceInfoNum);
	}

	/** Initialize Visualization Data */
	CollapsedTileVisualInfo = nullptr;
}


FIntVector FWFC3DCell::IndexToLocation(const int32 Index, const FIntVector& Dimension)
{
	return {Index % Dimension.X, Index / Dimension.X % Dimension.Y, Index / Dimension.X / Dimension.Y};
}

bool FWFC3DCell::IsFacePropagated(const EFace& Direction) const
{
	return PropagatedFaces & 1 << FWFC3DFaceUtils::GetIndex(Direction);
}

void FWFC3DCell::SetPropagatedFaces(const EFace& Direction)
{
	PropagatedFaces |= 1 << FWFC3DFaceUtils::GetIndex(Direction);
}
