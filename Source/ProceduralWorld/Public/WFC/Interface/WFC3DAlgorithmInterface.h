// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "WFC/Model/WFC3DTypes.h"
#include "WFC3DAlgorithmInterface.generated.h"

// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class UWFC3DAlgorithmInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 3D 웨이브 함수 붕괴(Wave Function Collapse) 알고리즘 인터페이스
 * 이 인터페이스는 WFC 알고리즘의 기본 기능을 정의합니다.
 * 
 * @note 이 인터페이스를 구현하는 클래스는 WFC 알고리즘의 핵심 기능을 제공해야 합니다.
 */
class PROCEDURALWORLD_API IWFC3DAlgorithmInterface
{
	GENERATED_BODY()

public:
	/** 모델 데이터 초기화 */
	virtual bool InitializeAlgorithmData() = 0;

	/** 타일 정보 배열 가져오기 */
	virtual const TArray<FTileInfo>& GetTileInfos() const = 0;

	/** 면 정보 배열 가져오기 */
	virtual const TArray<FFaceInfo>& GetFaceInfos() const = 0;

	/** 특정 면에 대해 호환되는 타일의 비트맵 가져오기 */
	virtual const TBitArray<>& GetCompatibleTiles(const int32& FaceIndex) const = 0;

	/** 특정 타일의 면 정보 가져오기 */
	virtual const FTileFaceIndices& GetTileFaces(const int32& TileIndex) const = 0;

	/** 특정 타일의 가중치 가져오기 */
	virtual const float& GetTileWeight(const int32& TileIndex) const = 0;
};
