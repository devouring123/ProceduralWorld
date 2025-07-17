// Fill out your copyright notice in the Description page of Project Settings.


#include "WFC/Algorithm/WFC3DAlgorithmTypes.h"

TArray<int32> FWFC3DHelperFunctions::GetAllIndexFromBitset(const TBitArray<>& Bitset)
{
	TArray<int32> Result;
	int32 Index = 0;
	do
	{
		Index = Bitset.FindFrom(true, Index);
		if (Index != INDEX_NONE)
		{
			Result.Add(Index++);
		}
	}
	while (Index != INDEX_NONE);
	return Result;
}

int32 FWFC3DHelperFunctions::GetWeightedRandomIndex(const TArray<float>& Weights, const FRandomStream* RandomStream)
{
	float TotalWeight = 0.0f;
	for (const float Weight : Weights)
	{
		TotalWeight += Weight;
	}

	float RandomValue = RandomStream->FRandRange(0.0f, TotalWeight);
	for (int32 i = 0; i < Weights.Num(); ++i)
	{
		RandomValue -= Weights[i];
		if (RandomValue <= 0.0f)
		{
			return i;
		}
	}

	return -1; // Should not reach here
}
