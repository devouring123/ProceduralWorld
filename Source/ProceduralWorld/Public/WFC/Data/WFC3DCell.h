// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "WFC3DCell.generated.h"
#include "WFC/Data/WFC3DTypes.h"

/**
 * WFC 알고리즘의 셀 구조체
 */
USTRUCT(BlueprintType)
struct PROCEDURALWORLD_API FWFC3DCell
{
	GENERATED_BODY()

public:

	FWFC3DCell() = default;
	
	void FWFC3DCell::Init(const int32 TileInfoNum, const int32 FaceInfoNum, const int32 Index, const FIntVector& Dimension);

	/** Common Data */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "WFC3D")
	bool bIsCollapsed = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "WFC3D")
	bool bIsPropagated = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "WFC3D")
	FIntVector Location;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "WFC3D")
	FTileInfo* CollapsedTileInfo = nullptr;

	/** Algorithm Data */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "WFC3D")
	int32 Entropy = 0;

	TBitArray<> RemainingTileOptionsBitset;
	
	TArray<TBitArray<>> MergedFaceOptionsBitset;
	
private:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "WFC3D")
	uint8 PropagatedFaces = 0;
	
public:
	/** Visualization Data */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "WFC3D")
	FTileVisualInfo* CollapsedTileVisualInfo = nullptr;

	/** Function */
	static FORCEINLINE FIntVector&& IndexToLocation(const int32 Index, const FIntVector& Dimension);

	bool FORCEINLINE IsFacePropagated(const EFace& Direction) const;
	
	void FORCEINLINE SetPropagatedFaces(const EFace& Direction);
};
