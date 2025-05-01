// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "WFC3DCell.generated.h"
/**
 * 
 */
USTRUCT(BlueprintType)
struct PROCEDURALWORLD_API FWFC3DCell
{
	GENERATED_BODY()

	FWFC3DCell() = default;

	void Init();

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "WFC3D")
	bool bIsCollapsed = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "WFC3D")
	bool bIsPropagated = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "WFC3D")
	FIntVector Location;


	
	

	
};
