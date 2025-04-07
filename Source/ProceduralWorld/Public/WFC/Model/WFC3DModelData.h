// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "WFC3DTypes.h"
#include "WFC3DModelData.generated.h"

// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class UWFC3DModelData : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class PROCEDURALWORLD_API IWFC3DModelData
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
	// 모델 데이터 초기화
	virtual bool Initialize() = 0;

	// 타일 정보 배열 가져오기
	virtual const TArray<FTileInfo>& GetTileInfos() const = 0;

	// 면 정보 배열 가져오기
	virtual const TArray<FFaceInfo>& GetFaceInfos() const = 0;

	// 특정 면에 대해 호환되는 타일의 비트맵 가져오기
	virtual TBitArray<> GetCompatibleTiles(int32 FaceIndex) const = 0;

	// 특정 타일의 면 정보 가져오기
	virtual FTileFaceIndices GetTileFaces(int32 TileIndex) const = 0;
};
