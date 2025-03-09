// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "WFC3DModel.h"
#include "GameFramework/Actor.h"
#include "WFC3DDataGenerator.generated.h"

UCLASS()
class PROCEDURALWORLD_API AWFC3DDataGenerator : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	AWFC3DDataGenerator();

	virtual void OnConstruction(const FTransform& Transform) override;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "WFC3D|Data")
	UWFC3DModel* WFC3DModel;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "WFC3D|Data")
	bool bCreateModelData = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "WFC3D|Data")
	bool bPrintModelData = false;
	
private:

	bool CreateModelData() const;
	bool PrintModelData() const;
};
