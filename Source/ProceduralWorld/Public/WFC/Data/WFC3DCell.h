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

	bool FORCEINLINE IsFacePropagated(const EFace& Direction) const;
	void FORCEINLINE SetPropagatedFaces(const EFace& Direction);
	static FORCEINLINE FIntVector IndexToLocation(const int32 Index, const FIntVector& Dimension);

	void PrintCellInfo() const
	{
		UE_LOG(LogTemp, Log, TEXT("Cell Location: %s, IsCollapsed: %s, IsPropagated: %s, Entropy: %d"),
			*Location.ToString(),
			bIsCollapsed ? TEXT("True") : TEXT("False"),
			bIsPropagated ? TEXT("True") : TEXT("False"),
			Entropy);
	}
	
public:
	/** Common Data */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "WFC3D")
	FIntVector Location;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "WFC3D")
	bool bIsCollapsed = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "WFC3D")
	bool bIsPropagated = false;

	const FTileInfo* CollapsedTileInfo = nullptr;

	/** Algorithm Data */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "WFC3D")
	int32 Entropy = 0;

	TBitArray<> RemainingTileOptionsBitset;

	TArray<TBitArray<>> MergedFaceOptionsBitset;

	/** Visualization Data */
	FTileVisualInfo* CollapsedTileVisualInfo = nullptr;

private:
	UPROPERTY(EditAnywhere, Category = "WFC3D")
	uint8 PropagatedFaces = 0;
};
