// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "WFC3DTypes.h"

/**
 * WFC3D 모델에서 Face와 관련된 유틸리티 함수들을 제공하는 클래스
 * 기존 FFaceUtils 클래스의 기능을 확장하여 더 많은 Face 관련 작업을 수행합니다.
 */
class PROCEDURALWORLD_API FWFC3DFaceUtils
{
public:
	/**
	 * 주어진 Face의 반대 방향 Face를 반환합니다.
	 * @param Face - 반대 방향을 찾을 Face
	 * @return 반대 방향의 Face, 유효하지 않은 입력이면 EFace::None 반환
	 */
	static FORCEINLINE EFace GetOpposite(const EFace& Face);

	/**
	 * Face를 정수 인덱스로 변환합니다.
	 * @param Face - 변환할 Face
	 * @return Face의 인덱스
	 */
	static FORCEINLINE uint8 GetIndex(const EFace& Face);

	/**
	 * Face의 반대 방향 인덱스를 반환합니다.
	 * @param Face - 반대 방향 인덱스를 찾을 Face
	 * @return 반대 방향 Face의 인덱스
	 */
	static FORCEINLINE uint8 GetOppositeIndex(const EFace& Face);

	/**
	 * 지정된 Face 방향의 방향 벡터를 반환합니다.
	 * @param Face - 방향 벡터를 찾을 Face
	 * @return Face 방향의 단위 벡터, 유효하지 않은 입력이면 (0,0,0) 반환
	 */
	static FORCEINLINE FIntVector GetDirectionVector(const EFace& Face);

	/**
	 * Face를 지정된 스텝만큼 회전합니다.
	 * @param Face - 회전할 Face
	 * @param Step - 회전 스텝 (0-3)
	 * @return 회전된 Face
	 */
	static FORCEINLINE EFace Rotate(const EFace& Face, const uint8& Step);

	/**
	 * UD(Up/Down) Face 문자열을 회전시킵니다.
	 * @param FaceName - 회전할 Face 문자열
	 * @param RotationSteps - 회전 스텝 (0-3)
	 * @return 회전된 Face 문자열
	 */
	static FString RotateUDFace(const FString& FaceName, int32 RotationSteps);

	/**
	 * BRLF(Back/Right/Left/Front) Face 문자열을 반전시킵니다.
	 * @param FaceName - 반전할 Face 문자열
	 * @return 반전된 Face 문자열
	 */
	static FString FlipBRLFFace(const FString& FaceName);

	/**
	 * 반대 방향 Face를 리턴합니다.
	 * @param Face - 반대 방향을 찾을 Face
	 * @return 반전된 Face 정보
	 */
	static FFaceInfo GetOppositeFace(const FFaceInfo& Face);

	// TODO : Delete this function
	/**
	 * Face 배열에서 주어진 Face와 일치하는 Face가 있는지 확인합니다.
	 * @param FaceIndex - 확인할 Face 정보
	 * @param FaceIndices - Face 문자열 배열
	 * @return 일치하는 Face가 있으면 true, 아니면 false
	 */
	static bool HasMatchingFace(int32 FaceIndex, const TArray<int32>& FaceIndices);

	/**
	 * 모든 방향(Up, Back, Right, Left, Front, Down)을 표현하는 상수 배열
	 * for 루프 등에서 모든 방향을 순회할 때 사용할 수 있습니다.
	 * 예시: for(const EFace& Face : FWFC3DFaceUtils::AllDirections) { ... }
	 */
	static constexpr EFace AllDirections[6] = {
		EFace::Up,
		EFace::Back,
		EFace::Right,
		EFace::Left,
		EFace::Front,
		EFace::Down
	};

private:
	/** 각 방향에 대한 방향 벡터 배열 */
	static const FIntVector DirectionVectors[6];

	/** Face 회전 매핑 테이블 */
	static constexpr EFace RotationMap[6][4] = {
		{EFace::Up, EFace::Up, EFace::Up, EFace::Up}, // Up
		{EFace::Back, EFace::Left, EFace::Front, EFace::Right}, // Back
		{EFace::Right, EFace::Back, EFace::Left, EFace::Front}, // Right
		{EFace::Left, EFace::Front, EFace::Right, EFace::Back}, // Left
		{EFace::Front, EFace::Right, EFace::Back, EFace::Left}, // Front
		{EFace::Down, EFace::Down, EFace::Down, EFace::Down} // Down
	};
};

// 인라인 함수 구현
FORCEINLINE EFace FWFC3DFaceUtils::GetOpposite(const EFace& Face)
{
	const uint8 Index = static_cast<uint8>(Face);
	return Index < 6 ? static_cast<EFace>(5 - Index) : EFace::None;
}

FORCEINLINE uint8 FWFC3DFaceUtils::GetIndex(const EFace& Face)
{
	return static_cast<uint8>(Face);
}

FORCEINLINE uint8 FWFC3DFaceUtils::GetOppositeIndex(const EFace& Face)
{
	return 5 - static_cast<uint8>(Face);
}

FORCEINLINE FIntVector FWFC3DFaceUtils::GetDirectionVector(const EFace& Face)
{
	const uint8 Index = static_cast<uint8>(Face);
	return Index < 6 ? DirectionVectors[Index] : FIntVector(0, 0, 0);
}

FORCEINLINE EFace FWFC3DFaceUtils::Rotate(const EFace& Face, const uint8& Step)
{
	const uint8 Index = static_cast<uint8>(Face);
	return (Index < 6 && Step < 4) ? RotationMap[Index][Step] : EFace::None;
}
