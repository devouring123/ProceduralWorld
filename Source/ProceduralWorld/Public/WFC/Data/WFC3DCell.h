// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "WFC/Data/WFC3DTypes.h"
#include "WFC3DCell.generated.h"

/**
 * WFC 알고리즘의 셀 구조체
 */
USTRUCT(BlueprintType)
struct PROCEDURALWORLD_API FWFC3DCell
{
	GENERATED_BODY()

public:

	FWFC3DCell() = default;
	
	void Initialize(const int32 TileInfoNum, const int32 FaceInfoNum, const int32 Index, const FIntVector& Dimension);

	/** Common Data */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "WFC3D")
	bool bIsCollapsed = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "WFC3D")
	bool bIsPropagated = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "WFC3D")
	FIntVector Location;
	
	FTileInfo* CollapsedTileInfo = nullptr;

	/** Algorithm Data */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "WFC3D")
	int32 Entropy = 0;

	TBitArray<> RemainingTileOptionsBitset;
	
	TArray<TBitArray<>> MergedFaceOptionsBitset;
	
private:
	UPROPERTY(EditAnywhere, Category = "WFC3D")
	uint8 PropagatedFaces = 0;
	
public:
	/** Visualization Data */
	FTileVisualInfo* CollapsedTileVisualInfo = nullptr;

	/** Function */
	static FORCEINLINE FIntVector IndexToLocation(const int32 Index, const FIntVector& Dimension);

	bool FORCEINLINE IsFacePropagated(const EFace& Direction) const;
	
	void FORCEINLINE SetPropagatedFaces(const EFace& Direction);
};
