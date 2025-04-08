// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "WFC3DAlgorithmInterface.h"
#include "WFC3DVisualizationInterface.h"
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
};
