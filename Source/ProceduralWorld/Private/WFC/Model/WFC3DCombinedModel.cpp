// Fill out your copyright notice in the Description page of Project Settings.


#include "WFC/Model/WFC3DCombinedModel.h"
#include "WFC/Model/WFC3DModelDataAsset.h"

bool UWFC3DCombinedModel::InitializeModelData()
{
	if (!ModelDataAsset)
	{
		UE_LOG(LogTemp, Error, TEXT("ModelDataAsset is nullptr"));
		return false;
	}

	if (!ModelDataAsset->InitializeData())
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to Initialize Model Data"));
		return false;
	}

	AlgorithmInterface = ModelDataAsset;
	VisualizationInterface = ModelDataAsset;

	bIsInitialized = true;
	return true;
}

TScriptInterface<IWFC3DAlgorithmInterface> UWFC3DCombinedModel::GetAlgorithmInterface() const
{
	if (!bIsInitialized)
	{
		UE_LOG(LogTemp, Error, TEXT("ModelDataAsset is not Initialized"));
		return nullptr;
	}
	return AlgorithmInterface;
}

TScriptInterface<IWFC3DVisualizationInterface> UWFC3DCombinedModel::GetVisualizationInterface() const
{
	if (!bIsInitialized)
	{
		UE_LOG(LogTemp, Error, TEXT("ModelDataAsset is not Initialized"));
		return nullptr;
	}
	return VisualizationInterface;
}
