// Fill out your copyright notice in the Description page of Project Settings.

#include "WFC/Model/WFC3DFaceUtils.h"

// 방향 벡터 초기화
const FIntVector FWFC3DFaceUtils::DirectionVectors[6] = {
	FIntVector(0, 0, 1), // Up
	FIntVector(0, -1, 0), // Back
	FIntVector(-1, 0, 0), // Right
	FIntVector(1, 0, 0), // Left
	FIntVector(0, 1, 0), // Front
	FIntVector(0, 0, -1) // Down
};

FString FWFC3DFaceUtils::RotateUDFace(const FString& Face, int32 RotationSteps)
{
	FRegexPattern Pattern(TEXT("([+-]?\\d+)([a-zA-Z])"));
	FRegexMatcher Matcher(Pattern, Face);

	if (Matcher.FindNext())
	{
		FString NumberPart = Matcher.GetCaptureGroup(1);
		FString CharPart = Matcher.GetCaptureGroup(2);

		if (CharPart.IsEmpty())
		{
			return Face;
		}

		// 문자 순환 처리 ('a' -> 'b' -> 'c' -> 'd' -> 'a')
		TCHAR CurrentChar = CharPart[0];
		TCHAR NextChar = (CurrentChar - 'a' + RotationSteps) % 4 + 'a';

		return NumberPart + FString(1, &NextChar);
	}

	return Face;
}

FTileInfo FWFC3DFaceUtils::RotateTileClockwise(const FTileInfo& TileInfo,
                                               const TArray<FFaceInfo>& FaceInfos,
                                               const TMap<FFaceInfo,int32>& FaceInfoToIndex,
                                               const int32& RotationStep)
{
	FTileInfo NewTileInfo(TileInfo);

	// UD(Up/Down) 회전
	NewTileInfo.Faces[GetIndex(EFace::Up)] = FaceInfoToIndex[FFaceInfo( EFace::Up, RotateUDFace(FaceInfos[TileInfo.Faces[GetIndex(EFace::Up)]].Name, RotationStep))];
	NewTileInfo.Faces[GetIndex(EFace::Down)] = FaceInfoToIndex[FFaceInfo( EFace::Down, RotateUDFace(FaceInfos[TileInfo.Faces[GetIndex(EFace::Down)]].Name, RotationStep))];

	// BRLF(Back/Right/Left/Front) 회전
	for (uint8 i = 1; i < 5; ++i)
    {
        NewTileInfo.Faces[i] = TileInfo.Faces[GetIndex(Rotate(static_cast<EFace>(i), RotationStep))];
    }

	return NewTileInfo;
}

bool FWFC3DFaceUtils::HasMatchingFace(const FFaceInfo& FaceInfo, const TArray<FString>& Faces)
{
	// Face 쌍과 인덱스 추출
	const uint8 OppositeIndex = GetOppositeIndex(FaceInfo.Direction);

	// UD(Up/Down) Face 매칭 확인
	if (FaceInfo.Direction == EFace::Up || FaceInfo.Direction == EFace::Down)
	{
		return Faces[OppositeIndex] == FaceInfo.Name;
	}

	// BRLF(Back/Right/Left/Front) Face 매칭 확인
	if (FaceInfo.Direction >= EFace::Back && FaceInfo.Direction <= EFace::Front)
	{
		// 대칭 면 확인 (symmetrical face)
		// Face1 == "3s", Face2 == "3s" => true
		if (Faces[OppositeIndex] == FaceInfo.Name &&
			Faces[OppositeIndex].Contains(TEXT("s")) &&
			FaceInfo.Name.Contains(TEXT("s")))
		{
			return true;
		}

		// 암-수 커넥터 확인 (female-male connector)
		// Face1 == "2f", Face2 == "2" => true
		// Face1 == "2", Face2 == "2f" => true
		if (Faces[OppositeIndex] == FaceInfo.Name + TEXT("f") ||
			Faces[OppositeIndex] + TEXT("f") == FaceInfo.Name)
		{
			return true;
		}
	}

	return false;
}
