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

	/** Algorithm Interface */
	virtual bool InitializeAlgorithmData() override;
	virtual const TArray<FTileInfo>& GetTileInfos() const override;
	virtual const TArray<FFaceInfo>& GetFaceInfos() const override;
	virtual const TBitArray<>& GetCompatibleTiles(const int32& FaceIndex) const override;
	virtual const FTileFaceIndices& GetTileFaces(const int32& TileIndex) const override;
	virtual const float& GetTileWeight(const int32& TileIndex) const override;
	/** End Algorithm Interface */

	/** Visualization Interface */
	virtual bool InitializeVisualizationData() override;
	virtual const TArray<FTileRotationInfo>& GetTileRotationInfo() const override;
	virtual const FTileVariantInfo& GetTileVariant(const int32& TileIndex) const override;
	virtual const FTileVisualInfo& GetRandomTileVisualInfo(const int32& BaseTileIndex, const FString& BiomeName) const override;
	/** End Visualization Interface */

private:

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "WFC3D|Data")
	TObjectPtr<UDataTable> TileInfoTable = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "WFC3D|Data")
	TObjectPtr<UDataTable> TileVariantInfoTable = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "WFC3D|Data")
	TArray<FTileInfo> TileInfos;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "WFC3D|Data")
	TArray<FFaceInfo> FaceInfos;

	/** 명명을 조금 더 자세히 할 것 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "WFC3D|Data")
	TArray<FBitString> FaceToTileBitStringArrays;
	
	TArray<TBitArray<>> FaceToTileBitArrays;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "WFC3D|Data")
	TArray<FTileVariantInfo> Variants;

};
