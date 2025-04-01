// Fill out your copyright notice in the Description page of Project Settings.


#include "WFC/WFC3DActor.h"

void FCell3D::Init(int32 TileInfoNum, int32 FaceInfoNum, const int32& Index, const FIntVector& Dimension)
{
	IsCollapsed = false;
	IsPropagated = false;
	UE_LOG(LogTemp, Display, TEXT("Index: %d"), Index);
	Location = IndexToLocation(Index, Dimension);
	CollapsedTileInfo = nullptr;
	Entropy = TileInfoNum;
	PropagatedFaces = {false, false, false, false, false, false};
	RemainingTileOptionsBitset.Init(true, TileInfoNum);
	for (int32 Direction = 0; Direction < 6; ++Direction)
	{
		MergedFaceOptionsBitset[Direction].Init(true, FaceInfoNum);
	}
}

FIntVector&& FCell3D::IndexToLocation(const int32& Index, const FIntVector& Dimension)
{
	// index -> 3차원 위치로 변환
	return {Index % Dimension.X, Index / Dimension.X % Dimension.Y, Index / Dimension.X / Dimension.Y};
}

FGrid::FGrid(const int32& X, const int32& Y, const int32& Z)
{
	Dimension = FIntVector(X, Y, Z);
	Grid.Init(FCell3D(), X * Y * Z);
	RandomStream.Initialize(RandomSeed);
}

FGrid::FGrid(const FIntVector& Dimension): Dimension(Dimension)
{
	Grid.Init(FCell3D(), Dimension.X * Dimension.Y * Dimension.Z);
	RandomStream.Initialize(RandomSeed);
}

FGrid& FGrid::operator=(const FGrid& Other)
{
	Grid = Other.Grid;
	Dimension = Other.Dimension;
	WFC3DModel = Other.WFC3DModel;

	return *this;
}

void FGrid::Init(UWFC3DModel* InWFCModel, const FIntVector& InDimension)
{
	WFC3DModel = InWFCModel;
	Dimension = InDimension;

	if (!IsValid(WFC3DModel))
	{
		UE_LOG(LogTemp, Display, TEXT("WFC3DModel is not Valid"));
		return;
	}
	Grid.Init(FCell3D(), Dimension.X * Dimension.Y * Dimension.Z);

	for (int32 Index = 0; Index < Dimension.X * Dimension.Y * Dimension.Z; ++Index)
	{
		Grid[Index].Init(WFC3DModel->TileInfos.Num(), WFC3DModel->FaceInfos.Num(), Index, Dimension);
	}
	RemainingCells = Dimension.X * Dimension.Y * Dimension.Z;
}


bool FGrid::GetCell(const FIntVector& Location, FCell3D& OutCell)
{
	if (!CheckLocation(Location))
	{
		return false;
	}
	OutCell = Grid[Location.X + Location.Y * Dimension.X + Location.Z * Dimension.X * Dimension.Y];
	return true;
}


bool FGrid::CollapseGrid()
{
	UE_LOG(LogTemp, Display, TEXT("CollapseGrid"));
	UE_LOG(LogTemp, Display, TEXT("RemainingCells: %d"), RemainingCells);
	while (RemainingCells > 0)
	{
		int32 LowestEntropy = INT32_MAX;
		for (const auto& Cell : Grid)
		{
			if (Cell.Entropy < LowestEntropy && !Cell.IsCollapsed)
			{
				LowestEntropy = Cell.Entropy;
			}
		}

		TArray<int32> CellsWithLowestEntropy;
		for (int32 i = 0; i < Grid.Num(); ++i)
		{
			if (Grid[i].Entropy == LowestEntropy && !Grid[i].IsCollapsed)
			{
				CellsWithLowestEntropy.Add(i);
			}
		}

		int32 SelectedGridIndex = CellsWithLowestEntropy[Rand0(CellsWithLowestEntropy.Num())];
		FIntVector CollapseLocation = Grid[SelectedGridIndex].Location;

		TArray<int32> TileInfoIndex = GetAllIndexFromBitset(Grid[SelectedGridIndex].RemainingTileOptionsBitset);
		int32 SelectedTileInfoIndex = TileInfoIndex[Rand0(TileInfoIndex.Num())];
		FBaseTileInfo* TileInfo = &WFC3DModel->TileInfos[SelectedTileInfoIndex];

		if (!CollapseCell(CollapseLocation, TileInfo))
		{
			UE_LOG(LogTemp, Display, TEXT("Failed to CollapseCell"));
			return false;
		}

		for (int32 Direction = 0; Direction < 6; ++Direction)
		{
			FIntVector NextLocation = CollapseLocation + PropagateDirection[Direction];
			FCell3D CellToPropagate;
			if (GetCell(NextLocation, CellToPropagate) && !CellToPropagate.IsCollapsed)
			{
				PropagationQueue.Enqueue(NextLocation);
				CellToPropagate.IsPropagated = true;
			}
		}

		PropagateGrid();

		for (auto& Cell : Grid)
		{
			if (!Cell.IsCollapsed)
			{
				Cell.IsPropagated = false;
			}
		}
	}
	return true;
}

bool FGrid::CollapseCell(const FIntVector& Location, FBaseTileInfo* TileInfo)
{
	FCell3D CollapsedCell;

	UE_LOG(LogTemp, Display, TEXT("CollapseCell Location : %d, %d, %d"),
	       Location.X, Location.Y, Location.Z);
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

bool FGrid::PropagateGrid()
{
	while (!PropagationQueue.IsEmpty())
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

bool FGrid::PropagateCell(const FIntVector& Location)
{
	UE_LOG(LogTemp, Display, TEXT("PropagateCell Location : %d, %d, %d"),
	       Location.X, Location.Y, Location.Z);

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
		TArray<int32> MergedFaceIndices = GetAllIndexFromBitset(
			PropagatedCellInDirection.MergedFaceOptionsBitset[5 - Direction]);
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
	TArray<TBitArray<>> MergedFaceOptionsBitset = {
		TBitArray<>(), TBitArray<>(), TBitArray<>(), TBitArray<>(), TBitArray<>(), TBitArray<>()
	};
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

bool FGrid::CheckLocation(FIntVector Location) const
{
	return !(Location.X < 0 || Location.X >= Dimension.X || Location.Y < 0 || Location.Y >= Dimension.Y || Location.Z < 0 || Location.Z >= Dimension.Z);
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

const TArray<FIntVector> FGrid::PropagateDirection = {
	{0, 0, 1},
	{0, 1, 0},
	{1, 0, 0},
	{-1, 0, 0},
	{0, -1, 0},
	{0, 0, -1},
};

// TODO: Actor에서 실제로 Propagate 하기
AWFC3DActor::AWFC3DActor()
{
	PrimaryActorTick.bCanEverTick = false;
}

void AWFC3DActor::Init()
{
	Grid.Init(WFC3DModel, Dimension);
}

void AWFC3DActor::Collapse()
{
	Grid.CollapseGrid();
}

void AWFC3DActor::OnConstruction(const FTransform& Transform)
{
	if (bInitGrid)
	{
		Init();
		UE_LOG(LogTemp, Display, TEXT("Init Success"));
		bInitGrid = false;
	}

	if (bCollapseGrid)
	{
		bool bResult = Grid.CollapseGrid();
		UE_LOG(LogTemp, Display, TEXT("Collapse Grid : %hs"), bResult ? "true" : "false");
		bCollapseGrid = false;
	}
}

void AWFC3DActor::BeginPlay()
{
	Super::BeginPlay();

	Grid = FGrid(Dimension);
	Init();
	Collapse();
}
