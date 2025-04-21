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
	
	/** Algorithm Interface */
	virtual bool InitializeAlgorithmData() override;
	virtual const TArray<FTileInfo>* GetTileInfos() const override;
	virtual const TArray<FFaceInfo>* GetFaceInfos() const override;
	virtual const TBitArray<>* GetCompatibleTiles(const int32& FaceIndex) const override;
	virtual const TArray<int32>* GetTileFaceIndices(const int32& TileIndex) const override;
	virtual const float GetTileWeight(const int32& TileIndex) const override;
	/** End Algorithm Interface */

	/** Visualization Interface */
	virtual bool InitializeVisualizationData() override;
	virtual const TArray<FTileRotationInfo>* GetTileRotationInfo() const override;
	virtual const FTileVariantInfo* GetTileVariant(const int32& TileIndex) const override;
	virtual const FTileVisualInfo* GetRandomTileVisualInfo(const int32& BaseTileIndex, const FString& BiomeName) const override;
	/** End Visualization Interface */

private:
	
	/** Initialize */
	bool InitializeCommonData();

	/**
	 *	초기화 담당
	 *  근데 조금 순서가 꼬인 것 같음
	 *  FaceInfo에 회전한 UDFace들이 누락됨, 이걸 어떻게 하지?
	 *  1. FaceInfo를 초기화 할 때 회전한 UDFace들을 추가한다.
	 *  2. TileInfo를 초기화 할 때 누락된 회전한 UDFace들을 추가한다.
	 *		- 이건 함수가 하는 일에 맞지 않음
	 *		
	 */
	bool InitializeBaseTileInfo();
	bool InitializeTileInfo();
	bool InitializeFaceInfo();

	bool InitializeFaceToTileBitArray();
	bool InitializeFaceToTileBitString();
	
	bool InitializeTileVariantInfo();
	bool InitializeTileRotationInfo();

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
	TArray<FTileInfo> TileInfos;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "WFC3D|Data")
	TArray<FFaceInfo> FaceInfos;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "WFC3D|Data")
	TMap<FFaceInfo, int32> FaceInfoToIndex;
	
	
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
