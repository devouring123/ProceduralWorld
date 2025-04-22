// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "WFC/Interface/WFC3DAlgorithmInterface.h"
#include "WFC/Interface/WFC3DVisualizationInterface.h"
#include "Engine/DataAsset.h"
#include "WFC3DModelDataAsset.generated.h"

/**
 * 
 */
UCLASS()
class PROCEDURALWORLD_API UWFC3DModelDataAsset : public UDataAsset,
                                                 public IWFC3DAlgorithmInterface,
                                                 public IWFC3DVisualizationInterface
{
	GENERATED_BODY()

public:
	/** Constructor */
	UWFC3DModelDataAsset() = default;

	UWFC3DModelDataAsset::UWFC3DModelDataAsset(UDataTable* InBaseTileDataTable, UDataTable* InTileVariantDataTable)
	{
		BaseTileDataTable = InBaseTileDataTable;
		TileVariantDataTable = InTileVariantDataTable;
	}

	/** Initialize */
	bool InitializeData();
	bool InitializeCommonData();
	
	/** Algorithm Interface */
	virtual bool InitializeAlgorithmData() override;
	virtual const TArray<FTileInfo>* GetTileInfos() const override;
	virtual const TArray<FFaceInfo>* GetFaceInfos() const override;
	virtual const TBitArray<>* GetCompatibleTiles(int32 FaceIndex) const override;
	virtual const TArray<int32>* GetTileFaceIndices(int32 TileIndex) const override;
	virtual const float GetTileWeight(int32 TileIndex) const override;
	/** End Algorithm Interface */

	/** Visualization Interface */
	virtual bool InitializeVisualizationData() override;
	virtual const TArray<FTileRotationInfo>* GetTileRotationInfo() const override;
	virtual const FTileVariantInfo* GetTileVariant(int32 TileIndex) const override;
	virtual const FTileVisualInfo* GetRandomTileVisualInfo(int32 BaseTileIndex, const FString& BiomeName) const override;
	/** End Visualization Interface */

private:
	
	/** Initialize */
	bool InitializeBaseTileInfo();
	bool InitializeFaceInfo();
	bool InitializeTileInfo();

	/** FaceIndex -> TileIndex */
	bool InitializeFaceToTileBitStringAndBitArray();

	bool InitializeTileVariantInfo();
	bool InitializeTileRotationInfo();

	/**
	 * 주어진 타일 정보를 지정된 스텝만큼 시계 방향으로 회전시킵니다.
	 * @param TileInfo - 회전할 타일 정보
	 * @param RotationStep - 회전 스텝 (0-3)
	 * @return 회전된 타일 정보
	 */
	FTileInfo RotateTileClockwise(const FTileInfo& TileInfo, int32 RotationStep);

	/** Data Table */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "WFC3D|Data")
	TObjectPtr<UDataTable> BaseTileDataTable = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "WFC3D|Data")
	TObjectPtr<UDataTable> TileVariantDataTable = nullptr;

	/** Base Tile Data */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "WFC3D|Data")
	TMap<FString, int32> BaseTileNameToIndex;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "WFC3D|Data")
	TArray<FBaseTileInfo> BaseTileInfos;

	/** Common Data */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "WFC3D|Data")
	TArray<FFaceInfo> FaceInfos;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "WFC3D|Data")
	TArray<FTileInfo> TileInfos;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "WFC3D|Data")
	TMap<FFaceInfo, int32> FaceToIndex;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "WFC3D|Data")
	TMap<int32, int32> OppositeFaceIndex;

	/** Algorithm Data */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "WFC3D|Data")
	TArray<FBitString> FaceToTileBitStrings;

	TArray<TBitArray<>> FaceToTileBitArrays;

	/** Visualization Data */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "WFC3D|Data")
	TArray<FTileVariantInfo> TileVariants;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "WFC3D|Data")
	TArray<FTileRotationInfo> TileRotationInfos;
};
