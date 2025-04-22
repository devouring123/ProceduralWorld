// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "WFC/Model/WFC3DTypes.h"
#include "WFC3DVisualizationInterface.generated.h"

// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class UWFC3DVisualizationInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 3D 웨이브 함수 붕괴(Wave Function Collapse) 시각화 인터페이스
 * 이 인터페이스는 WFC 알고리즘의 결과를 시각화하기 위한 기능을 정의합니다.
 * 
 * @note 이 인터페이스를 구현하는 클래스는 타일의 시각적 정보를 제공해야 합니다.
 */
class PROCEDURALWORLD_API IWFC3DVisualizationInterface
{
	GENERATED_BODY()

public:
	/** 모델 데이터 초기화 */
	virtual bool InitializeVisualizationData() = 0;

	/** 모든 타일 회전 정보 가져오기 */
	virtual const TArray<FTileRotationInfo>* GetTileRotationInfo() const = 0;

	/** 타일 변형 정보 가져오기 */
	virtual const FTileVariantInfo* GetTileVariant(int32 BaseTileIndex) const = 0;

	/** 랜덤 타일 가져오기 */
	virtual const FTileVisualInfo* GetRandomTileVisualInfo(int32 BaseTileIndex, const FString& BiomeName) const = 0;
	
};
