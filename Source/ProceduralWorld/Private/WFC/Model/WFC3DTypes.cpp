// Fill out your copyright notice in the Description page of Project Settings.

#include "WFC/Model/WFC3DTypes.h"

void FTileByBiome::CalculateTotalWeight()
{
	// Calculate the total weight of all variants
	TotalWeight = 0.0f;
	for (const FTileVisualInfo& Tile : Tiles)
	{
		TotalWeight += Tile.Weight;
	}
}

void FTileVariantInfo::CalculateTotalWeight()
{
	// Calculate the total weight of all variants
	for (TTuple<FString, FTileByBiome>& Biome : Biomes)
	{
		Biome.Value.CalculateTotalWeight();
	}
}