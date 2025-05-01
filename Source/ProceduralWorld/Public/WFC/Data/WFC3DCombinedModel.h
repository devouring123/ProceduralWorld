// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "WFC/Interface/WFC3DAlgorithmInterface.h"
#include "WFC3DCombinedModel.generated.h"

class IWFC3DVisualizationInterface;
class IWFC3DAlgorithmInterface;
class UWFC3DModelDataAsset;
/**
 * 
 */
UCLASS()
class PROCEDURALWORLD_API UWFC3DCombinedModel : public UObject
{
	GENERATED_BODY()

public:
	UWFC3DCombinedModel() = default;

	UWFC3DCombinedModel(UWFC3DModelDataAsset* InModelDataAsset)
	{
		ModelDataAsset = InModelDataAsset;
	}

	bool InitializeModelData();
	TScriptInterface<IWFC3DAlgorithmInterface> GetAlgorithmInterface() const;
	TScriptInterface<IWFC3DVisualizationInterface> GetVisualizationInterface() const;

private:
	UPROPERTY(EditAnywhere, Category = "WFC3D|Data")
	TObjectPtr<UWFC3DModelDataAsset> ModelDataAsset;
	
	TScriptInterface<IWFC3DAlgorithmInterface> AlgorithmInterface;
	TScriptInterface<IWFC3DVisualizationInterface> VisualizationInterface;

	bool bIsInitialized = false;
};
