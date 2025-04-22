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

FString FWFC3DFaceUtils::RotateUDFace(const FString& FaceName, int32 RotationSteps)
{
	FRegexPattern Pattern(TEXT("([+-]?\\d+)([a-zA-Z])"));
	FRegexMatcher Matcher(Pattern, FaceName);

	if (Matcher.FindNext())
	{
		FString NumberPart = Matcher.GetCaptureGroup(1);
		FString CharPart = Matcher.GetCaptureGroup(2);

		if (CharPart.IsEmpty())
		{
			return FaceName;
		}

		// 문자 순환 처리 ('a' -> 'b' -> 'c' -> 'd' -> 'a')
		TCHAR CurrentChar = CharPart[0];
		TCHAR NextChar = (CurrentChar - 'a' + RotationSteps) % 4 + 'a';

		return NumberPart + FString(1, &NextChar);
	}

	return FaceName;
}

FString FWFC3DFaceUtils::FlipBRLFFace(const FString& FaceName)
{
	if (FaceName.IsEmpty())
	{
		return TEXT("NONE");
	}
    
	const int32 NameLength = FaceName.Len();
    
	if (NameLength > 0 && FaceName[NameLength - 1] == 's')
	{
		return FaceName;
	}
    
	if (NameLength > 0 && FaceName[NameLength - 1] == 'f')
	{
		return FaceName.LeftChop(1);
	}
    
	return FaceName + TEXT("f");
}


FFaceInfo FWFC3DFaceUtils::GetOppositeFace(const FFaceInfo& Face)
{
	const EFace OppositeDirection = GetOpposite(Face.Direction);
    
	if (Face.Direction == EFace::Up || Face.Direction == EFace::Down)
	{
		return FFaceInfo(OppositeDirection, Face.Name);
	}
	return FFaceInfo(OppositeDirection, FlipBRLFFace(Face.Name));
}
