// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "WFC3DFaceUtils.h"
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

	UWFC3DModelDataAsset(UDataTable* InBaseTileDataTable, UDataTable* InTileVariantDataTable)
	{
		BaseTileDataTable = InBaseTileDataTable;
		TileVariantDataTable = InTileVariantDataTable;
	}

	/** Initialize */
	bool InitializeData();
	bool PrintData();

	/** Algorithm Interface */
	virtual bool InitializeAlgorithmData() override;
	virtual const TArray<FTileInfo>* GetTileInfos() const override;
	virtual const FTileInfo* GetTileInfo(int32 TileIndex) const override;
	virtual const int32 GetTileInfosNum() const override;
	virtual const TArray<FFaceInfo>* GetFaceInfos() const override;
	virtual const FFaceInfo* GetFaceInfo(int32 FaceIndex) const override;
	virtual const int32 GetFaceInfosNum() const override;
	virtual const TBitArray<>* GetCompatibleTiles(int32 FaceIndex) const override;
	virtual const TArray<int32>* GetTileFaceIndices(int32 TileIndex) const override;
	virtual const float GetTileWeight(int32 TileIndex) const override;
	/** End Algorithm Interface */

	/** Visualization Interface */
	virtual bool InitializeVisualizationData() override;
	virtual const TArray<FTileRotationInfo>* GetTileRotationInfos() const override;
	virtual const FTileRotationInfo* GetTileRotationInfo(int32 TileIndex) const override;
	virtual const FTileVariantInfo* GetTileVariant(int32 TileIndex) const override;
	virtual const FTileVisualInfo* GetRandomTileVisualInfo(int32 BaseTileIndex, const FString& BiomeName) const override;
	/** End Visualization Interface */

private:
	/** Initialize */
	bool InitializeCommonData();
	bool InitializeBaseTileInfo();
	bool InitializeFaceInfo();
	bool InitializeTileInfo();
	bool InitializeFaceToTile();
	bool InitializeTileVariantInfo();

	bool LoadFaceToTileBitArrays();


	/**
	 * 주어진 타일 정보를 지정된 스텝만큼 시계 방향으로 회전시킵니다.
	 * @param TileInfo - 회전할 타일 정보
	 * @param RotationStep - 회전 스텝 (0-3)
	 * @return 회전된 타일 정보
	 */
	FTileInfo RotateTileClockwise(const FTileInfo& TileInfo, int32 RotationStep);

	/** Data Table */
	UPROPERTY(EditAnywhere, Category = "WFC3D|Data")
	TObjectPtr<UDataTable> BaseTileDataTable = nullptr;

	UPROPERTY(EditAnywhere, Category = "WFC3D|Data")
	TObjectPtr<UDataTable> TileVariantDataTable = nullptr;

	/** Base Tile Data */
	UPROPERTY(EditAnywhere, Category = "WFC3D|Data")
	TArray<FBaseTileInfo> BaseTileInfos;

	UPROPERTY(EditAnywhere, Category = "WFC3D|Data")
	TArray<FString> BaseTileNames;

	UPROPERTY(EditAnywhere, Category = "WFC3D|Data")
	TMap<FString, int32> BaseTileNameToIndex;

	/** Common Data */
	UPROPERTY(EditAnywhere, Category = "WFC3D|Data")
	TArray<FFaceInfo> FaceInfos;

	UPROPERTY(EditAnywhere, Category = "WFC3D|Data")
	TMap<FFaceInfo, int32> FaceToIndex;

	UPROPERTY(EditAnywhere, Category = "WFC3D|Data")
	TArray<int32> OppositeFaceIndices;

	UPROPERTY(EditAnywhere, Category = "WFC3D|Data")
	TArray<FTileInfo> TileInfos;

	/** Algorithm Data */
	UPROPERTY(EditAnywhere, Category = "WFC3D|Data")
	TArray<FBitString> FaceToTileBitStrings;

	TArray<TBitArray<>> FaceToTileBitArrays;

	/** Visualization Data */
	UPROPERTY(EditAnywhere, Category = "WFC3D|Data")
	TArray<FTileVariantInfo> TileVariants;

	UPROPERTY(EditAnywhere, Category = "WFC3D|Data")
	TArray<FTileRotationInfo> TileRotationInfos;


	/** Debug */
	bool PrintFaceInfos()
	{
		UE_LOG(LogTemp, Display, TEXT("FaceInfo Size: %d"), FaceInfos.Num());
		for (int32 Index = 0; Index < FaceInfos.Num(); ++Index)
		{
			UE_LOG(LogTemp, Display, TEXT("FaceInfo %d: (%s, %s)"), Index,
			       *StaticEnum<EFace>()->GetNameStringByValue((int64)FaceInfos[Index].Direction),
			       *FaceInfos[Index].Name);
		}
		return true;
	}

	bool PrintTileInfos()
	{
		UE_LOG(LogTemp, Display, TEXT("TileInfos Size: %d"), TileInfos.Num());
		for (int32 Index = 0; Index < TileInfos.Num(); ++Index)
		{
			UE_LOG(LogTemp, Display, TEXT("TileInfo %d: BaseTileID: %d"), Index, TileInfos[Index].BaseTileID);
			for (int32 FaceIndex = 0; FaceIndex < TileInfos[Index].Faces.Num(); ++FaceIndex)
			{
				UE_LOG(LogTemp, Display, TEXT("    TileInfo %d: %s: %s"), Index,
					*StaticEnum<EFace>()->GetNameStringByValue((int64)FWFC3DFaceUtils::AllDirections[FaceIndex]),
					*FaceInfos[TileInfos[Index].Faces[FaceIndex]].Name);
			}
			UE_LOG(LogTemp, Display, TEXT("TileInfo %d: Weight: %f"), Index, TileInfos[Index].Weight);
		}
		return true;
	}

	bool PrintFaceToTileBitMap()
	{
		UE_LOG(LogTemp, Display, TEXT("FaceToTileBitArrays Size: %d"), FaceToTileBitArrays.Num());

		for (int32 Index = 0; Index < FaceToTileBitArrays.Num(); ++Index)
		{
			UE_LOG(LogTemp, Display, TEXT("FaceToTileBitArray %d : %s"), Index, *FBitString::ToString(FaceToTileBitArrays[Index]));
		}
		return true;
	}

	bool PrintTileVariants()
	{
		UE_LOG(LogTemp, Display, TEXT("TileVariants Size: %d"), TileVariants.Num());

		for (int32 VariantIndex = 0; VariantIndex < TileVariants.Num(); ++VariantIndex)
		{
			UE_LOG(LogTemp, Display, TEXT("TileName: %s"), *BaseTileNames[VariantIndex]);

			const FTileVariantInfo& TileVariantInfo = TileVariants[VariantIndex];
			for (auto& Biome : TileVariantInfo.Biomes)
			{
				UE_LOG(LogTemp, Display, TEXT("    Biome: %s"), *Biome.Key);
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
		return true;
	}
};
