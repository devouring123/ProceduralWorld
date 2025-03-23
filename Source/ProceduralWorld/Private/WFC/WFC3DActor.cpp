// Fill out your copyright notice in the Description page of Project Settings.


#include "WFC/WFC3DActor.h"

bool FGrid::CollapseCell(const FIntVector& Location, FBaseTileInfo* TileInfo)
{
	FCell3D CollapsedCell;

	if (!GetCell(Location, CollapsedCell))
	{
		UE_LOG(LogTemp, Display, TEXT("Failed to GetCell in CollapseCell"));
		return false;
	}

	CollapsedCell.CollapsedTileInfo = TileInfo;
	CollapsedCell.Entropy = 1;
	CollapsedCell.RemainingTileOptionsBitset.Empty();

	CollapsedCell.IsCollapsed = true;
	CollapsedCell.IsPropagated = true;

	RemainingCells--;
	
	return true;
}

bool FGrid::PropagateCell(const FIntVector& Location)
{
	if (!CheckLocation(Location))
	{
		UE_LOG(LogTemp, Display, TEXT("Not In Grid in PropagateCell"));
		return false;
	}

	FCell3D PropagatedCell;
	if (!GetCell(Location, PropagatedCell))
	{
		return false;
	}

	for (int32 Direction = 0; Direction < 6; ++Direction)
	{
		// 전파 받은 면이 아니면 넘기기
		if (!PropagatedCell.PropagatedFaces[Direction])
		{
			continue;
		}

		// 전파 받은 방향의 면이 있다면 해당 방향으로 전파 받은 면의 타일 옵션을 가져옴
		FCell3D PropagatedCellInDirection;
		FIntVector FacedLocation = Location + PropagateDirection[Direction];
		GetCell(FacedLocation, PropagatedCellInDirection);

		// 전파 받은 방향에서 해당 방향으로 전파받은 모든 타일 옵션을 가져와서 OR 연산
		TBitArray<> MergedTileOptionsForDirection;
		MergedTileOptionsForDirection.Init(false, WFC3DModel->TileInfos.Num());

		// PropagatedCellInDirection.MergedFaceOptionsBitset[5 - Direction]의 모든 비트에서 각 비트에 해당하는 TileBitset을 가져와서 OR 연산
		TArray<int32> MergedFaceIndices = GetAllIndexFromBitset(PropagatedCellInDirection.MergedFaceOptionsBitset[5 - Direction]);
		for (int32 Index : MergedFaceIndices)
		{
			MergedTileOptionsForDirection.CombineWithBitwiseOR(WFC3DModel->FaceToTileBitArrayMap[Index], EBitwiseOperatorFlags::MaintainSize);
		}

		// 나온 결과를 RemainingTileOptionsBitset과 And 연산
		PropagatedCell.RemainingTileOptionsBitset.CombineWithBitwiseAND(MergedTileOptionsForDirection, EBitwiseOperatorFlags::MaintainSize);
	}
	// 모든 방향에 대한 전파가 끝나면 남은 타일 셋을 확인해서 엔트로피 갱신
	PropagatedCell.Entropy = PropagatedCell.RemainingTileOptionsBitset.CountSetBits();

	// RemainingTileOptionBitset이 1이면 붕괴
	if (PropagatedCell.Entropy == 1)
	{
		// 붕괴
		FBaseTileInfo* TileInfo = &WFC3DModel->TileInfos[PropagatedCell.RemainingTileOptionsBitset.Find(true)];
		CollapseCell(Location, TileInfo);
	}

	// 남은 타일 옵션에 대해서 각 면에 대한 비트셋을 OR 연산해서 MergedFaceOptionsBitset에 넣음
	TArray<TBitArray<>> MergedFaceOptionsBitset = {TBitArray<>(), TBitArray<>(), TBitArray<>(), TBitArray<>(), TBitArray<>(), TBitArray<>()};
	for (int32 Direction = 0; Direction < 6; ++Direction)
	{
		MergedFaceOptionsBitset[Direction].Init(false, WFC3DModel->FaceInfos.Num());
	}

	// 남은 타일의 인덱스의 비트맵을 모두 합치기?
	// 남은 타일의 인덱스를 모두 가져옴
	TArray<int32> RemainingTileIndices = GetAllIndexFromBitset(PropagatedCell.RemainingTileOptionsBitset);
	for (int32 Index : RemainingTileIndices)
	{
		// 남은 타일에 대해서 각 면을 가져와서 해당 비트셋 true로 변경
		FTileFaceIndices FaceIndices = WFC3DModel->TileToFaceMap[Index];
		for (int32 Direction = 0; Direction < 6; ++Direction)
		{
			MergedFaceOptionsBitset[Direction][FaceIndices.FaceIndices[Direction]] = true;
		}
	}

	// MergedFaceOptionsBitset을 PropagatedCell에 넣음
	FCell3D CellToPropagate;
	for (int32 Direction = 0; Direction < 6; ++Direction)
	{
		// 만약 전파 받기 전과 전파 받은후가 같다면 더 이상 전파할 필요가 없음
		if (MergedFaceOptionsBitset[Direction] == PropagatedCell.MergedFaceOptionsBitset[Direction])
		{
			continue;
		}

		PropagatedCell.MergedFaceOptionsBitset[Direction] = MergedFaceOptionsBitset[Direction];
		FIntVector NextLocation = Location + PropagateDirection[Direction];

		// 위치 에러 or 이미 붕괴 or 이미 전파됨
		if (!GetCell(NextLocation, CellToPropagate) || CellToPropagate.IsCollapsed || CellToPropagate.IsPropagated)
		{
			continue;
		}

		// Propagate to Next Cell
		// 전파 받은 반대 방향에서 해당 방향으로 전파
		CellToPropagate.PropagatedFaces[5 - Direction] = true;
		PropagationQueue.Enqueue(NextLocation);
	}

	// 전파 완료
	PropagatedCell.IsPropagated = true;
	return true;
}

bool FGrid::CollapseGrid()
{
	while (RemainingCells > 0)
	{
		int32 LowestEntropy = INT32_MAX;
		for(const auto& Cell : Grid)
		{
			if(Cell.Entropy < LowestEntropy && !Cell.IsCollapsed)
			{
				LowestEntropy = Cell.Entropy;
			}
		}

		TArray<int32> CellsWithLowestEntropy;
		for(int32 i = 0; i < Grid.Num(); ++i)
		{
			if(Grid[i].Entropy == LowestEntropy && !Grid[i].IsCollapsed)
			{
				CellsWithLowestEntropy.Add(i);
			}
		}

		int32 SelectedGridIndex = CellsWithLowestEntropy[Rand0(CellsWithLowestEntropy.Num())];
		FIntVector CollapseLocation = Grid[SelectedGridIndex].Location;

		TArray<int32> TileInfoIndex = GetAllIndexFromBitset(Grid[SelectedGridIndex].RemainingTileOptionsBitset);
		int32 SelectedTileInfoIndex = TileInfoIndex[Rand0(TileInfoIndex.Num())];
		FBaseTileInfo* TileInfo = &WFC3DModel->TileInfos[SelectedTileInfoIndex];

		if(!CollapseCell(CollapseLocation, TileInfo))
		{
			UE_LOG(LogTemp, Display, TEXT("Failed to CollapseCell"));
			return false;
		}
		
		for(int32 Direction = 0; Direction < 6; ++Direction)
        {
            FIntVector NextLocation = CollapseLocation + PropagateDirection[Direction];
            if(CheckLocation(NextLocation))
            {
                PropagationQueue.Enqueue(NextLocation);
            }
        }
		
		PropagateGrid();
	}
	return true;
}


bool FGrid::PropagateGrid()
{
	while (PropagationQueue.IsEmpty() == false)
    {
        FIntVector Location;
        PropagationQueue.Dequeue(Location);
        if (!PropagateCell(Location))
        {
            UE_LOG(LogTemp, Display, TEXT("Failed to PropagateCell"));
            return false;
        }
    }
	return true;
}

bool FGrid::CreateRandomSeed()
{
	RandomSeed = RandomStream.RandRange(0, INT32_MAX - 1);
	RandomStream.Initialize(RandomSeed);
	UE_LOG(LogTemp, Display, TEXT("RandomSeed = %d"), RandomSeed);
	return true;
}

bool FGrid::SetRandomSeed(int32 Seed)
{
	RandomSeed = Seed;
	RandomStream.Initialize(RandomSeed);
	UE_LOG(LogTemp, Display, TEXT("RandomSeed = %d"), Seed);
	return true;
}

TArray<int32> FGrid::GetAllIndexFromBitset(const TBitArray<>& Bitset)
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


bool FGrid::CheckLocation(FIntVector Location) const
{
	if (Location.X < 0 || Location.X >= Dimension.X || Location.Y < 0 || Location.Y >= Dimension.Y || Location.Z < 0 || Location.Z >= Dimension.Z)
	{
		UE_LOG(LogTemp, Display, TEXT("Out of Range in Grid: X: %d, Y: %d, Z: %d"), Location.X, Location.Y, Location.Z);
		return false;
	}
	return true;
}

AWFC3DActor::AWFC3DActor()
{
	PrimaryActorTick.bCanEverTick = false;
	Grid = FGrid(0, 0, 0);
}

void AWFC3DActor::Collapse()
{
}

void AWFC3DActor::BeginPlay()
{
	Super::BeginPlay();
}
