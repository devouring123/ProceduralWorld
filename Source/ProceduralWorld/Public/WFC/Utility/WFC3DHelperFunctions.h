// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

/**
 * WFC3D 알고리즘에서 사용되는 유틸리티 함수 모음
 */
class FWFC3DHelperFunctions
{
public:
	/**
	 * 비트셋에서 모든 인덱스를 가져오는 함수
	 * @param Bitset - TBitArray<> 비트셋
	 * @return TArray<int32> - 비트셋에서 true인 모든 인덱스의 배열
	 */
	static TArray<int32> GetAllIndexFromBitset(const TBitArray<>& Bitset);

	/**
	 * 랜덤한 정수를 반환하는 함수
	 * @param Weights - 가중치 배열
	 * @param RandomStream - 랜덤 스트림
	 * @return int32 - 가중치에 따라 선택된 랜덤한 인덱스
	 */
	static int32 GetWeightedRandomIndex(const TArray<float>& Weights, const FRandomStream* RandomStream);

private:
	/** 유틸리티 클래스 생성자 및 소멸자 제거 */
	FWFC3DHelperFunctions() = delete;
	FWFC3DHelperFunctions(const FWFC3DHelperFunctions&) = delete;
	FWFC3DHelperFunctions& operator=(const FWFC3DHelperFunctions&) = delete;
	FWFC3DHelperFunctions(FWFC3DHelperFunctions&&) = delete;
	FWFC3DHelperFunctions& operator=(FWFC3DHelperFunctions&&) = delete;
	~FWFC3DHelperFunctions() = delete;
};
