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
	bool InitializeFaceToTile();
	bool InitializeTileVariantInfo();

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
	TArray<int32> OppositeFaceIndex;

	/** Algorithm Data */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "WFC3D|Data")
	TArray<FBitString> FaceToTileBitStrings;

	TArray<TBitArray<>> FaceToTileBitArrays;

	/** Visualization Data */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "WFC3D|Data")
	TArray<FTileVariantInfo> TileVariants;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "WFC3D|Data")
	TArray<FTileRotationInfo> TileRotationInfos;


	/** Debug */
	void PrintFaceInfos()
	{
		UE_LOG(LogTemp, Display, TEXT("FaceToTileBitMapKeys Size: %d"), FaceInfos.Num());
		for (uint8 i = 0; i < FaceInfos.Num(); ++i)
		{
			UE_LOG(LogTemp, Display, TEXT("FaceToTileBitMapKey %d: (%s, %s)"), i, 
				   *StaticEnum<EFace>()->GetNameStringByValue((int64)FaceInfos[i].Direction),
				   *FaceInfos[i].Name);
		}
	}

	void PrintFaceToTileBitMap()
	{
		UE_LOG(LogTemp, Display, TEXT("FaceToTileBitMap Size: %d"), FaceToTileBitArrays.Num());
		for (auto& Elem : FaceToTileBitArrays)
		{
			UE_LOG(LogTemp, Display, TEXT("FaceToTileBitMap Key: %d"), Elem.Key);
			UE_LOG(LogTemp, Display, TEXT("FaceToTileBitMap Value: %s"), *FBitString::ToString(Elem.Value));
		}
	}

	void PrintTileVariants()
	{
		UE_LOG(LogTemp, Display, TEXT("TileVariants Size: %d"), TileVariants.Num());
		for (auto& Elem : TileVariants)
		{
			UE_LOG(LogTemp, Display, TEXT("TileName: %s"), *Elem.Key.ToString());
			for (auto& Biome : Elem.Value.Biomes)
			{
				UE_LOG(LogTemp, Display, TEXT("    Biome: %s"), *Biome.Key.ToString());
				for (auto& Tile : Biome.Value.Tiles)
				{
					UE_LOG(LogTemp, Display, TEXT("        Tile StaticMesh: %s"), *Tile.StaticMesh->GetName());
					UE_LOG(LogTemp, Display, TEXT("        Tile Materials Size: %d"), Tile.Materials.Num());
					for (auto& Material : Tile.Materials)
					{
						UE_LOG(LogTemp, Display, TEXT("            Tile Material: %s"), *Material->GetName());
					}
					UE_LOG(LogTemp, Display, TEXT("        Tile Weight: %f"), Tile.Weight);
				}
				UE_LOG(LogTemp, Display, TEXT("    Biome TotalWeight: %f"), Biome.Value.TotalWeight);
			}
		}
	}
};
